#include "../../inc/network/evdsfetcher.hpp"
#include "../../inc/logger.hpp"
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QDateTime>

EvdsFetcher::EvdsFetcher(HttpManager *http_manager, QObject *parent)
    : QObject(parent), http_manager_(http_manager) {
    connect(http_manager_, &HttpManager::jsonFetched, this, &EvdsFetcher::onJsonFetched);
    connect(http_manager_, &HttpManager::jsonFetchFailed, this, &EvdsFetcher::onJsonFetchFailed);
}

void EvdsFetcher::fetchExchangeRate(QDate date) {
    QString startDate = date.addDays(-3).toString("dd-MM-yyyy");
    QString endDate = date.toString("dd-MM-yyyy");
    QString url = QString("%1%2&startDate=%3&endDate=%4&type=json&aggregationTypes=last")
                        .arg(API_END_POINT, SERIES_USD, startDate, endDate);
    http_manager_->fetchJsonData(url);
}

void EvdsFetcher::fetchInflationIndex(QDate date) {
    // Set the date range to the first day of the previous month
    QDate prevMonth = date.addMonths(-1);
    QDate firstDayPrevMonth(prevMonth.year(), prevMonth.month(), 1);
    QString startDate = firstDayPrevMonth.toString("dd-MM-yyyy");
    QString endDate = futureDate_.toString("dd-MM-yyyy");
    QString url = QString("%1%2&startDate=%3&endDate=%4&type=json&aggregationTypes=last")
                        .arg(API_END_POINT, SERIES_INFLATION, startDate, endDate);
    qDebug(logNetwork) << "Fetching symbols with query:" << url;
    http_manager_->fetchJsonData(url);
}

void EvdsFetcher::onJsonFetched(const std::shared_ptr<QJsonObject> &data, const QString &url) {
    // Only process EVDS responses
    if (!url.contains(REQUESTER_FILTER)) {
        qDebug(logNetwork) << "EvdsFetcher: Ignoring non-EVDS response from" << url;
        return;
    }
    
    if (data) {
        // Save received JSON data to a local file when debug mode is enabled
        if (logNetwork().isDebugEnabled()) {
            QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
            QDir dir;
            if (!dir.exists(dataDir)) {
                dir.mkpath(dataDir);
            }
            
            QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
            QString filePath = dataDir + QString("/evds_response_%1.txt").arg(timestamp);
            
            QFile file(filePath);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);
                QJsonDocument doc(*data);
                out << "EVDS API Response - " << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << "\n";
                out << "=" << QString(60, '=') << "\n\n";
                out << doc.toJson(QJsonDocument::Indented);
                file.close();
                qDebug(logNetwork) << "EvdsFetcher: Saved response to" << filePath;
            } else {
                qWarning(logNetwork) << "EvdsFetcher: Failed to save response to file:" << filePath;
            }
        }
        
        QString seriesCode;

        if (data->contains("items")) {
            QJsonArray items = data->value("items").toArray();
            qDebug(logNetwork) << "EvdsFetcher: Processing" << items.size() << "items";

            // Special condition: totalCount == 0 and items is empty
            // Occurs when last month's inflation index has not been published yet
            if (data->value("totalCount").toInt() == 0 && items.isEmpty()) {
                qDebug(logNetwork) << "EvdsFetcher: No data available (totalCount=0 and items empty)";
                emit fetchFailed("Henüz EVDS tarafından yayımlanmayan veri var.\nLütfen bir kaç gün sonra tekrar deneyin.");
                return;
            }

            // Check all items to determine the series type
            for (int i = 0; i < items.size(); ++i) {
                QJsonObject item = items.at(i).toObject();
                qDebug(logNetwork) << "EvdsFetcher: Item" << i << "keys:" << item.keys();

                // Check for USD series field
                if (item.contains("TP_DK_USD_A")) {
                    seriesCode = SERIES_USD;
                    qInfo(logNetwork) << "EvdsFetcher: Detected USD series";
                    break;
                }
                // Check for inflation series field  
                else if (item.contains("TP_TUFE1YI_T1")) {
                    seriesCode = SERIES_INFLATION;
                    qInfo(logNetwork) << "EvdsFetcher: Detected inflation series";
                    break;
                }
            }
        }
        else {
            qWarning(logNetwork) << "EvdsFetcher: JSON data does not contain 'items' array";
            emit fetchFailed("TCMB sunucusundan beklenmeyen veri formatı alındı.");

            return;
        }

        qInfo(logNetwork) << "EvdsFetcher: Emitting series code:" << seriesCode;
        emit evdsDataFetched(data, seriesCode);
    }
    else {
        qWarning(logNetwork) << "EvdsFetcher: Received null JSON data";
        emit fetchFailed("TCMB sunucusundan veri alınamadı,\nDaha sonra tekrar deneyin.");
    }
}

void EvdsFetcher::onJsonFetchFailed(const QString &error) {
    qWarning(logNetwork) << "EvdsFetcher: JSON fetch failed:" << error;
    emit fetchFailed(error);
}