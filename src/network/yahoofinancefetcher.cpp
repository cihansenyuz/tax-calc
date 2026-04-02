#include "../../inc/network/yahoofinancefetcher.hpp"
#include "../../inc/logger.hpp"
#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

YahooFinanceFetcher::YahooFinanceFetcher(QObject *parent)
    : QObject(parent), http_manager_(HttpManager::getInstance()) {
    connect(http_manager_, &HttpManager::jsonFetched, this, &YahooFinanceFetcher::onJsonFetched);
    connect(http_manager_, &HttpManager::jsonFetchFailed, this, &YahooFinanceFetcher::onJsonFetchFailed);
}

void YahooFinanceFetcher::fetchSymbols(const QString &query) {
    QString encodedQuery = QUrl::toPercentEncoding(query);
    QString url = QString("%1?q=%2&%3&%4").arg(apiEndPoint, encodedQuery, quotesField, newsCountField);
    qDebug(logNetwork) << "Fetching symbols with query:" << url;
    http_manager_->fetchJsonData(url);
}

void YahooFinanceFetcher::onJsonFetched(const std::shared_ptr<QJsonObject> &json, const QString &url) {
    if (!url.contains(REQUESTER_FILTER)) {
        qDebug(logNetwork) << "YahooFinanceFetcher: Ignoring non-Yahoo Finance response from" << url;
        return;
    }
    
    if (!json) {
        qWarning(logNetwork) << "YahooFinanceFetcher: Received null JSON data";
        emit fetchFailed("Yahoo Finance sunucusundan veri alınamadı,\nDaha sonra tekrar deneyin.");
        return;
    }

    QList<QPair<QString, QString>> results;
    if (json->contains("quotes") && (*json)["quotes"].isArray()) {
        qDebug(logNetwork) << "YahooFinanceFetcher: Processing quotes data";
        QJsonArray quotes = (*json)["quotes"].toArray();
        for (const QJsonValue &val : quotes) {
            if (val.isObject()) {
                QJsonObject quoteObj = val.toObject();
                QString symbol = quoteObj.value("symbol").toString();
                QString shortname = quoteObj.value("shortname").toString();
                if (!symbol.isEmpty() && !shortname.isEmpty()) {
                    results.append(qMakePair(symbol, shortname));
                }
            }
        }
    }

    qInfo(logNetwork) << "YahooFinanceFetcher: Emitting symbols";
    emit symbolsFetched(results);
}

void YahooFinanceFetcher::onJsonFetchFailed(const QString &error) {
    qWarning(logNetwork) << "YahooFinanceFetcher: JSON fetch failed:" << error;
    emit fetchFailed(error);
}
