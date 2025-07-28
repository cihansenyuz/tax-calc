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
    m_assets.push_back(asset);
    QString startDate = asset.getBuyDate();
    QString endDate = startDate;
    m_evds_fetcher->fetch(EvdsFetcher::USD, startDate, endDate);
}

void AssetManager::onEvdsDataFetched(const std::shared_ptr<QJsonObject> &data) {
    qDebug() << "AssetManager received fetched data";
    
    QFile file("fetched_data.json");
    
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(*data);
        file.write(doc.toJson());
        file.close();
    }
}
