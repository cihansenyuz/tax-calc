#include "../inc/transactionmanager.hpp"
#include "../inc/network/evdsfetcher.hpp"
#include "../inc/network/httpmanager.hpp"
#include "../inc/calculator.hpp"
#include <QDebug>

TransactionManager::TransactionManager(QObject *parent)
    : QObject(parent) {
    m_http_manager = HttpManager::getInstance();
    m_http_manager->setKey(EvdsFetcher::API_KEY);
    m_evds_fetcher = new EvdsFetcher(m_http_manager, this);
    connect(m_evds_fetcher, &EvdsFetcher::evdsDataFetched, this, &TransactionManager::onEvdsDataFetched);
    connect(m_evds_fetcher, &EvdsFetcher::fetchFailed, this, &TransactionManager::onFetchFailed);
    
    m_asset_db = & TransactionDatabase::getInstance("assets.db");
    if(!m_asset_db->initAssetTable())
        throw std::runtime_error("Failed to initialize asset database table");

    m_transactions = m_asset_db->loadAssets();
}

TransactionManager::~TransactionManager() {
    delete m_evds_fetcher;
}

void TransactionManager::openTransaction(const Transaction& transaction) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_transaction_to_be_updated = transaction;
    m_currentTransactionType = TransactionType::Open;
    m_exchangeRateReceived = false;
    m_inflationIndexReceived = false;
    m_data_to_be_updated = {0.0, 0.0}; // Reset data
    
    m_evds_fetcher->fetchExchangeRate(transaction.getBuyQDate());
    m_evds_fetcher->fetchInflationIndex(transaction.getBuyQDate());
}

void TransactionManager::processOpenTransaction() {
    std::unique_lock<std::mutex> lock(m_mutex);

    m_transaction_to_be_updated.setExchangeRateAtBuy(m_data_to_be_updated.first);
    m_transaction_to_be_updated.setInflationIndexAtBuy(m_data_to_be_updated.second);

    if(!m_asset_db->saveAsset(m_transaction_to_be_updated))
        throw std::runtime_error("Failed to save asset to database");

    m_transactions.push_back(m_transaction_to_be_updated);

    m_data_to_be_updated = {0.0, 0.0}; // Reset after use

    emit databaseReady();
}

void TransactionManager::closeTransaction(const Transaction& transaction) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_transaction_to_be_updated = transaction;
    m_currentTransactionType = TransactionType::Close;
    m_exchangeRateReceived = false;
    m_inflationIndexReceived = false;
    m_data_to_be_updated = {0.0, 0.0}; // Reset data

    m_evds_fetcher->fetchExchangeRate(transaction.getSellQDate());
    m_evds_fetcher->fetchInflationIndex(transaction.getSellQDate());
}

void TransactionManager::potentialTransaction(const Transaction& transaction) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_transaction_to_be_updated = transaction;
    m_currentTransactionType = TransactionType::Potential;
    m_exchangeRateReceived = false;
    m_inflationIndexReceived = false;
    m_data_to_be_updated = {0.0, 0.0}; // Reset data

    m_evds_fetcher->fetchExchangeRate(transaction.getSellQDate());
    m_evds_fetcher->fetchInflationIndex(transaction.getSellQDate());
}

void TransactionManager::processCloseTransaction() {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_transaction_to_be_updated.setExchangeRateAtSell(m_data_to_be_updated.first);
    m_transaction_to_be_updated.setInflationIndexAtSell(m_data_to_be_updated.second);

    m_transaction_to_be_updated.setTaxBase(Calculator::calculateTaxBase(m_transaction_to_be_updated));

    if(!m_asset_db->updateAsset(m_transaction_to_be_updated))
        throw std::runtime_error("Failed to update asset in database");

    for(auto& transaction : m_transactions){
        if (transaction.getId() == m_transaction_to_be_updated.getId()) {
            transaction = m_transaction_to_be_updated; // Update the existing transaction
            break;
        }
    }

    m_data_to_be_updated = {0.0, 0.0}; // Reset after use

    emit databaseReady();
}

void TransactionManager::processPotentialTransaction() {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_transaction_to_be_updated.setExchangeRateAtSell(m_data_to_be_updated.first);
    m_transaction_to_be_updated.setInflationIndexAtSell(m_data_to_be_updated.second);
    double potentialTaxBase = Calculator::calculateTaxBase(m_transaction_to_be_updated);

    m_data_to_be_updated = {0.0, 0.0}; // Reset after use
    emit potentialTaxBaseReady(potentialTaxBase);
}

void TransactionManager::onEvdsDataFetched(const std::shared_ptr<QJsonObject> &data,
                                    const QString &seriesCode) {
    // qDebug() << "EVDS data fetched for series:" << seriesCode;
    
    // // Thread-safe file writing with unique filename
    // static std::atomic<int> fileCounter{0};
    // QString filename = QString("fetched_data_%1_%2.json").arg(seriesCode).arg(fileCounter.fetch_add(1));
    // QFile file(filename);

    // if (file.open(QIODevice::WriteOnly)) {
    //     QJsonDocument doc(*data);
    //     file.write(doc.toJson());
    //     file.close();
    // }

    std::unique_lock<std::mutex> lock(m_mutex);
    
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
    
    if (m_exchangeRateReceived && m_inflationIndexReceived) {
        qDebug() << "Both values received, processing transaction";
        
        TransactionType currentType = m_currentTransactionType;
        
        m_currentTransactionType = TransactionType::None;
        m_exchangeRateReceived = false;
        m_inflationIndexReceived = false;
        lock.unlock();
        
        switch (currentType) {
            case TransactionType::Open:
                processOpenTransaction();
                break;
            case TransactionType::Close:
                processCloseTransaction();
                break;
            case TransactionType::Potential:
                processPotentialTransaction();
                break;
            case TransactionType::None:
                // This shouldn't happen due to early return above
                qWarning() << "Unexpected transaction state during processing";
                break;
        }
    }
}

Transaction TransactionManager::findTransactionById(int id) {
    std::unique_lock<std::mutex> lock(m_mutex);
    for (const auto& transaction : m_transactions) {
        if (transaction.getId() == id) {
            return transaction;
        }
    }
    throw std::runtime_error{"Asset with ID " + std::to_string(id) + " not found."};
}

void TransactionManager::removeTransaction(int id) {
    if(m_asset_db->deleteAsset(id)) {
        std::unique_lock<std::mutex> lock(m_mutex);
        auto it = std::remove_if(m_transactions.begin(), m_transactions.end(),
                                [id](const Transaction& transaction) { return transaction.getId() == id; });
        
        if (it != m_transactions.end()) {
            m_transactions.erase(it, m_transactions.end());
        }

        emit databaseReady();   
    }
    else {
        throw std::runtime_error{"Failed to delete asset with ID " + std::to_string(id) + " from database."};
    }
}

void TransactionManager::onFetchFailed(const QString &error) {
    m_currentTransactionType = TransactionType::None;
    m_exchangeRateReceived = false;
    m_inflationIndexReceived = false;
    m_data_to_be_updated = {0.0, 0.0};
    
    emit fetchFailed(error);
}