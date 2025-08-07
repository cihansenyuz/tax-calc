#include "../../inc/network/yahoofinancefetcher.hpp"
#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

YahooFinanceFetcher::YahooFinanceFetcher(HttpManager *http_manager, QObject *parent)
    : QObject(parent), http_manager_(http_manager) {
    connect(http_manager_, &HttpManager::jsonFetched, this, &YahooFinanceFetcher::onJsonFetched);
}

void YahooFinanceFetcher::fetchSymbols(const QString &query) {
    QString encodedQuery = QUrl::toPercentEncoding(query);
    QString url = QString("%1?q=%2&%3&%4").arg(apiEndPoint, encodedQuery, quotesField, newsCountField);
    qDebug() << "Fetching symbols with query:" << url;
    http_manager_->fetchJsonData(url);
}

void YahooFinanceFetcher::onJsonFetched(const std::shared_ptr<QJsonObject> &json) {
    if (!json) {
        emit fetchFailed("JSON fetch failed or is null");
        return;
    }
    QList<QPair<QString, QString>> results;
    if (json->contains("quotes") && (*json)["quotes"].isArray()) {
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
    emit symbolsFetched(results);
}
