#include "../inc/assetmanager.hpp"
#include "../inc/network/evdsfetcher.h"
#include "../inc/network/httpmanager.h"
#include "../inc/calculator.hpp"
#include <QDebug>

AssetManager::AssetManager(QObject *parent)
    : QObject(parent)
{
    m_http_manager = new HttpManager();
    m_http_manager->setKey(EvdsFetcher::API_KEY);
    m_evds_fetcher = new EvdsFetcher(m_http_manager, this);
    connect(m_evds_fetcher, &EvdsFetcher::evdsDataFetched, this, &AssetManager::onEvdsDataFetched);
    
    m_asset_db = & AssetDatabase::getInstance("assets.db");
    if(!m_asset_db->initAssetTable())
        throw std::runtime_error("Failed to initialize asset database table");

    m_assets = m_asset_db->loadAssets();
}

AssetManager::~AssetManager() {
    delete m_evds_fetcher;
    delete m_http_manager;
}

void AssetManager::openTransaction(const Asset& asset) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_asset_to_be_updated = asset;
    m_currentTransactionType = TransactionType::Open;
    m_exchangeRateReceived = false;
    m_inflationIndexReceived = false;
    m_data_to_be_updated = {0.0, 0.0}; // Reset data
    
    m_evds_fetcher->fetchExchangeRate(asset.getBuyQDate());
    m_evds_fetcher->fetchInflationIndex(asset.getBuyQDate());
}

void AssetManager::processOpenTransaction() {
    std::unique_lock<std::mutex> lock(m_mutex);

    m_asset_to_be_updated.setExchangeRateAtBuy(m_data_to_be_updated.first);
    m_asset_to_be_updated.setInflationIndexAtBuy(m_data_to_be_updated.second);
    
    m_assets.push_back(m_asset_to_be_updated);
    
    if(!m_asset_db->saveAsset(m_asset_to_be_updated))
        throw std::runtime_error("Failed to save asset to database");

    m_data_to_be_updated = {0.0, 0.0}; // Reset after use
    
    emit databaseReady();
}

void AssetManager::closeTransaction(const Asset& asset){
    std::unique_lock<std::mutex> lock(m_mutex);
    m_asset_to_be_updated = asset;
    m_currentTransactionType = TransactionType::Close;
    m_exchangeRateReceived = false;
    m_inflationIndexReceived = false;
    m_data_to_be_updated = {0.0, 0.0}; // Reset data

    m_evds_fetcher->fetchExchangeRate(asset.getSellQDate());
    m_evds_fetcher->fetchInflationIndex(asset.getSellQDate());
}

void AssetManager::processCloseTransaction(){
    std::unique_lock<std::mutex> lock(m_mutex);
    m_asset_to_be_updated.setExchangeRateAtSell(m_data_to_be_updated.first);
    m_asset_to_be_updated.setInflationIndexAtSell(m_data_to_be_updated.second);
    
    m_asset_to_be_updated.setTaxBase(Calculator::calculateTaxBase(m_asset_to_be_updated));

    for(auto& asset : m_assets){
        if (asset.getId() == m_asset_to_be_updated.getId()) {
            asset = m_asset_to_be_updated; // Update the existing asset
            break;
        }
    }

    if(!m_asset_db->updateAsset(m_asset_to_be_updated))
        throw std::runtime_error("Failed to update asset in database");

    m_data_to_be_updated = {0.0, 0.0}; // Reset after use
    
    emit databaseReady();
}

void AssetManager::onEvdsDataFetched(const std::shared_ptr<QJsonObject> &data, const QString &seriesCode) {
    qDebug() << "EVDS data fetched for series:" << seriesCode;
    
    // Thread-safe file writing with unique filename
    static std::atomic<int> fileCounter{0};
    QString filename = QString("fetched_data_%1_%2.json").arg(seriesCode).arg(fileCounter.fetch_add(1));
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(*data);
        file.write(doc.toJson());
        file.close();
    }

    std::unique_lock<std::mutex> lock(m_mutex);
    
    // Early return if no active transaction
    if (m_currentTransactionType == TransactionType::None) {
        qWarning() << "Received data with no active transaction";
        return;
    }
    
    if (data && data->contains("items")) {
        QJsonArray items = data->value("items").toArray();
        
        // Find the most recent non-null value
        double value = 0.0;
        for (int i = items.size() - 1; i >= 0; --i) {
            QJsonObject item = items.at(i).toObject();
            
            if (seriesCode == EvdsFetcher::SERIES_USD) {
                QJsonValue usdField = item.value("TP_DK_USD_A");
                if (!usdField.isNull()) {
                    if (usdField.isDouble()) {
                        value = usdField.toDouble();
                    } else if (usdField.isString()) {
                        value = usdField.toString().toDouble();
                    }
                    if (value > 0.0) {
                        m_data_to_be_updated.first = value;
                        m_exchangeRateReceived = true;
                        qDebug() << "Exchange rate received:" << value;
                        break;
                    }
                }
            } else if (seriesCode == EvdsFetcher::SERIES_INFLATION) {
                QJsonValue tufeField = item.value("TP_TUFE1YI_T1");
                if (!tufeField.isNull()) {
                    if (tufeField.isDouble()) {
                        value = tufeField.toDouble();
                    } else if (tufeField.isString()) {
                        value = tufeField.toString().toDouble();
                    }
                    if (value > 0.0) {
                        m_data_to_be_updated.second = value;
                        m_inflationIndexReceived = true;
                        qDebug() << "Inflation index received:" << value;
                        break;
                    }
                }
            }
        }
    }
    
    // Only proceed if both exchange rate and inflation index have been received
    if (m_exchangeRateReceived && m_inflationIndexReceived) {
        qDebug() << "Both values received, processing transaction";
        
        // Keep the lock held during the entire processing to ensure atomicity
        TransactionType currentType = m_currentTransactionType;
        
        // Reset transaction state before processing (still under lock)
        m_currentTransactionType = TransactionType::None;
        m_exchangeRateReceived = false;
        m_inflationIndexReceived = false;
        lock.unlock();
        
        // Process the transaction based on the captured type
        switch (currentType) {
            case TransactionType::Open:
                processOpenTransaction();
                break;
            case TransactionType::Close:
                processCloseTransaction();
                break;
            case TransactionType::None:
                // This shouldn't happen due to early return above
                qWarning() << "Unexpected transaction state during processing";
                break;
        }
    }
    
    // Lock is automatically released when going out of scope
}

Asset AssetManager::findAssetById(int id){
    std::unique_lock<std::mutex> lock(m_mutex);
    for (const auto& asset : m_assets) {
        if (asset.getId() == id) {
            return asset;
        }
    }
    throw std::runtime_error{"Asset with ID " + std::to_string(id) + " not found."};
}

void AssetManager::removeAsset(int id)
{
    if(m_asset_db->deleteAsset(id)) {
        std::unique_lock<std::mutex> lock(m_mutex);
        auto it = std::remove_if(m_assets.begin(), m_assets.end(),
                                [id](const Asset& asset) { return asset.getId() == id; });
        
        if (it != m_assets.end()) {
            m_assets.erase(it, m_assets.end());
        }

        emit databaseReady();   
    }
    else {
        throw std::runtime_error{"Failed to delete asset with ID " + std::to_string(id) + " from database."};
    }
}