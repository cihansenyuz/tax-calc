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
    
    m_asset_db = & AssetDatabase::getInstance("assets.db");
    m_asset_db->initAssetTable();
    m_assets = m_asset_db->loadAssets();
}

AssetManager::~AssetManager() {
    delete m_evds_fetcher;
    delete m_http_manager;
}

bool AssetManager::updateAssetInDb(const Asset& asset){
    return m_asset_db && m_asset_db->updateAsset(asset);
}

bool AssetManager::saveAssetToDb(const Asset& asset) {
    return m_asset_db && m_asset_db->saveAsset(asset);
}

bool AssetManager::loadAssetsFromDb() {
    if (!m_asset_db) return false;
    m_assets = m_asset_db->loadAssets();
    return true;
}

void AssetManager::openTransaction(const Asset& asset) {
    std::unique_lock<std::mutex> lock(m_mutex);
    
    m_asset_to_be_updated = asset;
    QString startDate = asset.getBuyDate();
    QString endDate = startDate;
    m_evds_fetcher->fetch(EvdsFetcher::SERIES, startDate, endDate);
    
    m_condition.wait(lock, [this](){
        return m_data_to_be_updated.first != 0.0 && m_data_to_be_updated.second != 0.0;
    });

    m_asset_to_be_updated.setExchangeRateAtBuy(m_data_to_be_updated.first);
    m_asset_to_be_updated.setInflationIndexAtBuy(m_data_to_be_updated.second);
    m_data_to_be_updated = {0.0, 0.0}; // Reset after use
    m_assets.push_back(m_asset_to_be_updated);
    m_asset_db->saveAsset(m_asset_to_be_updated);

    emit databaseReady();
}

void AssetManager::onEvdsDataFetched(const std::shared_ptr<QJsonObject> &data) {
    std::lock_guard<std::mutex> lock(m_mutex);
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

    m_data_to_be_updated = {usdValue, tufeValue};
    m_condition.notify_all();

    // QFile file("fetched_data.json");
    // if (file.open(QIODevice::WriteOnly)) {
    //     QJsonDocument doc(*data);
    //     file.write(doc.toJson());
    //     file.close();
    // }
}
