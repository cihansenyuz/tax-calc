#include "../../inc/network/evdsfetcher.hpp"
#include <QDebug>

EvdsFetcher::EvdsFetcher(HttpManager *http_manager, QObject *parent)
    : QObject(parent), http_manager_(http_manager) {
    connect(http_manager_, &HttpManager::jsonFetched, this, &EvdsFetcher::onJsonFetched);
}

void EvdsFetcher::fetchExchangeRate(QDate date) {
    QString startDate = date.addDays(-3).toString("dd-MM-yyyy");
    QString endDate = date.toString("dd-MM-yyyy");
    QString url = QString("%1%2&startDate=%3&endDate=%4&type=json&aggregationTypes=last")
                        .arg(API_END_POINT, SERIES_USD, startDate, endDate);
    http_manager_->fetchJsonData(url);
}

void EvdsFetcher::fetchInflationIndex(QDate date) {
    QString startDate = date.addMonths(-1).toString("dd-MM-yyyy");
    QString endDate = startDate;
    QString url = QString("%1%2&startDate=%3&endDate=%4&type=json&aggregationTypes=last")
                        .arg(API_END_POINT, SERIES_INFLATION, startDate, endDate);
    http_manager_->fetchJsonData(url);
}

void EvdsFetcher::onJsonFetched(const std::shared_ptr<QJsonObject> &data) {
    if (data) {
        QString seriesCode;

        if (data->contains("items")) {
            QJsonArray items = data->value("items").toArray();
            qDebug() << "EvdsFetcher: Processing" << items.size() << "items";

            // Special condition: totalCount == 0 and items is empty
            // Occurs when last month's inflation index has not been published yet
            if (data->value("totalCount").toInt() == 0 && items.isEmpty()) {
                emit fetchFailed("Henüz yayımlanmayan veri var.\nLütfen bir kaç gün sonra tekrar deneyin.");
                return;
            }

            // Check all items to determine the series type
            for (int i = 0; i < items.size(); ++i) {
                QJsonObject item = items.at(i).toObject();
                qDebug() << "EvdsFetcher: Item" << i << "keys:" << item.keys();

                // Check for USD series field
                if (item.contains("TP_DK_USD_A")) {
                    seriesCode = SERIES_USD;
                    qDebug() << "EvdsFetcher: Detected USD series";
                    break;
                }
                // Check for inflation series field  
                else if (item.contains("TP_TUFE1YI_T1")) {
                    seriesCode = SERIES_INFLATION;
                    qDebug() << "EvdsFetcher: Detected inflation series";
                    break;
                }
            }
        }

        qDebug() << "EvdsFetcher: Emitting series code:" << seriesCode;
        emit evdsDataFetched(data, seriesCode);
    }
    else {
        emit fetchFailed("TCMB sunucusundan veri alınamadı.");
    }
}