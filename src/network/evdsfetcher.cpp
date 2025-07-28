#include "../../inc/network/evdsfetcher.h"
#include <QDebug>

EvdsFetcher::EvdsFetcher(HttpManager *http_manager, QObject *parent)
    : QObject(parent), http_manager_(http_manager)
{
    connect(http_manager_, &HttpManager::JsonFetched, this, &EvdsFetcher::onJsonFetched);
}

void EvdsFetcher::fetch(const QString &series, const QString &startDate, const QString &endDate)
{
    QString query = QString("%1&startDate=%2&endDate=%3&type=json&aggregationTypes=last")
                        .arg(series, startDate, endDate);
    http_manager_->FetchJsonData(query);
}

void EvdsFetcher::onJsonFetched(const std::shared_ptr<QJsonObject> &data)
{
    if (data) {
        emit evdsDataFetched(data);
    } else {
        emit fetchFailed("Failed to fetch EVDS data");
    }
}
