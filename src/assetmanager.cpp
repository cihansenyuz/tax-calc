#include "../inc/assetmanager.hpp"
#include "../inc/network/evdsfetcher.h"
#include "../inc/network/httpmanager.h"
#include <QDebug>

AssetManager::AssetManager(QObject *parent)
    : QObject(parent)
{
    m_http_manager = new HttpManager();
    m_http_manager->setKey(API_KEY);
    m_evds_fetcher = new EvdsFetcher(m_http_manager, this);
    connect(m_evds_fetcher, &EvdsFetcher::evdsDataFetched, this, &AssetManager::onEvdsDataFetched);
}

AssetManager::~AssetManager() {
    delete m_evds_fetcher;
    delete m_http_manager;
}

void AssetManager::addAsset(const Asset& asset) {
    m_asset_to_be_updated = asset;
    QString startDate = asset.getBuyDate();
    QString endDate = startDate;
    m_evds_fetcher->fetch(EvdsFetcher::SERIES, startDate, endDate);
}

void AssetManager::onEvdsDataFetched(const std::shared_ptr<QJsonObject> &data) {
    qDebug() << "AssetManager received fetched data";

    double usdValue = 0.0;
    double tufeValue = 0.0;
    if (data && data->contains("items")) {
        QJsonArray items = data->value("items").toArray();
        if (!items.isEmpty()) {
            QJsonObject firstItem = items.at(0).toObject();

            QJsonValue usdField = firstItem.value("TP_DK_USD_A");
            if (!usdField.isNull() && usdField.isDouble()) {
                usdValue = usdField.toDouble();
            } else if (!usdField.isNull() && usdField.isString()) {
                usdValue = usdField.toString().toDouble();
            }

            QJsonValue tufeField = firstItem.value("TP_TUFE1YI_T1");
            if (!tufeField.isNull() && tufeField.isDouble()) {
                tufeValue = tufeField.toDouble();
            } else if (!tufeField.isNull() && tufeField.isString()) {
                tufeValue = tufeField.toString().toDouble();
            }
        }
    }

    m_asset_to_be_updated.setExchangeRate(usdValue);
    m_asset_to_be_updated.setInflationIndex(tufeValue);

    m_assets.push_back(m_asset_to_be_updated);
    emit assetUpdated();

    QFile file("fetched_data.json");
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(*data);
        file.write(doc.toJson());
        file.close();
    }
}
