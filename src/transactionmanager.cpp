#include "../inc/transactionmanager.hpp"
#include "../inc/network/evdsfetcher.hpp"
#include "../inc/network/httpmanager.hpp"
#include "../inc/calculator.hpp"
#include "../inc/logger.hpp"
#include <QSettings>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

QString TransactionReporter::actionToString(TransactionManager::TransactionType action) {
    switch (action) {
        case TransactionManager::TransactionType::Open:      return "Open Position";
        case TransactionManager::TransactionType::Close:     return "Close Position";
        case TransactionManager::TransactionType::Potential: return "Potential Tax Base Calculation";
        default:                                              return "Unknown";
    }
}

QString TransactionReporter::buildReportText(const Transaction& transaction,
                                              TransactionManager::TransactionType action,
                                              const FetchResult& buyData,
                                              const FetchResult& sellData,
                                              const TaxCalculationBreakdown& breakdown) {
    QString text;
    QTextStream out(&text);

    out << "=== Tax Calculation Report ===\n";
    if (action == TransactionManager::TransactionType::Potential)
        out << "*** POTENTIAL CALCULATION - NOT SAVED ***\n";
    out << "Transaction ID: " << transaction.getId() << " (" << QString::fromStdString(transaction.getSymbol()) << ")\n";
    out << "Action: " << actionToString(action) << "\n";
    out << "Generated: " << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << "\n\n";

    out << "Buy date: " << transaction.getBuyDate() << "\n";
    out << "  Exchange rate (USD/TRY): " << Calculator::formatCurrency(buyData.m_exchangeRate.value)
        << " (recorded when position was opened)\n";
    out << "  Inflation index: " << Calculator::formatCurrency(buyData.m_inflationIndex.value)
        << " (recorded when position was opened)\n";

    // Sell side and tax base breakdown only apply once a sell date/price exist
    if (action != TransactionManager::TransactionType::Open) {
        out << "\nSell date: " << transaction.getSellDate() << "\n";
        out << "  Exchange rate (USD/TRY): " << Calculator::formatCurrency(sellData.m_exchangeRate.value)
            << " (data date: "
            << (sellData.m_exchangeRate.date.isValid() ? sellData.m_exchangeRate.date.toString("dd-MM-yyyy") : QString("unknown"))
            << ")\n";
        out << "  Inflation index: " << Calculator::formatCurrency(sellData.m_inflationIndex.value)
            << " (data date: "
            << (sellData.m_inflationIndex.date.isValid() ? sellData.m_inflationIndex.date.toString("dd-MM-yyyy") : QString("unknown"))
            << ")\n\n";

        out << "Sell price (TRY): " << Calculator::formatCurrency(breakdown.sellPriceTry) << "\n";
        out << "Buy price (TRY): " << Calculator::formatCurrency(breakdown.buyPriceTry) << "\n";
        out << "Inflation scaler: " << QString::number(breakdown.inflationScaler, 'f', 4)
            << (breakdown.inflationAdjustmentApplied
                    ? " (>= 1.10 threshold -> inflation adjustment applied)\n"
                    : " (< 1.10 threshold -> inflation adjustment NOT applied)\n");
        out << "Tax base: " << Calculator::formatCurrency(breakdown.taxBase) << "\n";
        if (!breakdown.error.isEmpty())
            out << "Calculation error: " << breakdown.error << "\n";
    }

    return text;
}

void TransactionReporter::writeReport(const Transaction& transaction,
                                       TransactionManager::TransactionType action,
                                       const FetchResult& buyData,
                                       const FetchResult& sellData,
                                       const TaxCalculationBreakdown& breakdown) {
    QDir().mkpath("reports");

    QString actionTag = action == TransactionManager::TransactionType::Open      ? "open"
                      : action == TransactionManager::TransactionType::Close     ? "close"
                                                                                  : "potential";
    QString filename = QString("reports/%1_%2_%3.txt")
        .arg(transaction.getId())
        .arg(actionTag)
        .arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"));

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning(logManager) << "Failed to write transaction report:" << filename;
        return;
    }
    QTextStream stream(&file);
    stream << buildReportText(transaction, action, buyData, sellData, breakdown);
}

TransactionManager::TransactionManager(QObject *parent)
    : QObject(parent) {
    m_http_manager = HttpManager::getInstance();
    {
        QSettings settings;
        m_http_manager->setKey(settings.value("evds/api_key").toString());
    }
    m_evds_fetcher = new EvdsFetcher(m_http_manager, this);
    connect(m_evds_fetcher, &EvdsFetcher::evdsDataFetched, this, &TransactionManager::onEvdsDataFetched);
    connect(m_evds_fetcher, &EvdsFetcher::fetchFailed, this, &TransactionManager::onFetchFailed);
    
    m_asset_db = & TransactionDatabase::getInstance("assets.db");
    if(!m_asset_db->initAssetTable())
        throw std::runtime_error("Failed to initialize asset database table");

    m_transactions = m_asset_db->getAssetsFromDB();
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
    m_data_to_be_updated = {}; // Reset data
    
    m_evds_fetcher->fetchExchangeRate(transaction.getBuyQDate());
    m_evds_fetcher->fetchInflationIndex(transaction.getBuyQDate());
}

void TransactionManager::processOpenTransaction() {
    std::unique_lock<std::mutex> lock(m_mutex);

    m_transaction_to_be_updated.setExchangeRateAtBuy(m_data_to_be_updated.m_exchangeRate.value);
    m_transaction_to_be_updated.setInflationIndexAtBuy(m_data_to_be_updated.m_inflationIndex.value);

    if(!m_asset_db->saveAsset(m_transaction_to_be_updated))
        throw std::runtime_error("Failed to save asset to database");

    m_transactions.push_back(m_transaction_to_be_updated);

    TransactionReporter::writeReport(m_transaction_to_be_updated, TransactionType::Open,
                                      m_data_to_be_updated, FetchResult{}, TaxCalculationBreakdown{});

    m_data_to_be_updated = {}; // Reset after use

    emit databaseReady();
}

void TransactionManager::closeTransaction(const Transaction& transaction) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_transaction_to_be_updated = transaction;
    m_currentTransactionType = TransactionType::Close;
    m_exchangeRateReceived = false;
    m_inflationIndexReceived = false;
    m_data_to_be_updated = {}; // Reset data

    m_evds_fetcher->fetchExchangeRate(transaction.getSellQDate());
    m_evds_fetcher->fetchInflationIndex(transaction.getSellQDate());
}

void TransactionManager::potentialTransaction(const Transaction& transaction) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_transaction_to_be_updated = transaction;
    m_currentTransactionType = TransactionType::Potential;
    m_exchangeRateReceived = false;
    m_inflationIndexReceived = false;
    m_data_to_be_updated = {}; // Reset data

    m_evds_fetcher->fetchExchangeRate(transaction.getSellQDate());
    m_evds_fetcher->fetchInflationIndex(transaction.getSellQDate());
}

void TransactionManager::processCloseTransaction() {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_transaction_to_be_updated.setExchangeRateAtSell(m_data_to_be_updated.m_exchangeRate.value);
    m_transaction_to_be_updated.setInflationIndexAtSell(m_data_to_be_updated.m_inflationIndex.value);

    TaxCalculationBreakdown breakdown = Calculator::calculateTaxBaseDetailed(m_transaction_to_be_updated);
    m_transaction_to_be_updated.setTaxBase(breakdown.taxBase);

    if(!m_asset_db->updateAsset(m_transaction_to_be_updated))
        throw std::runtime_error("Failed to update asset in database");

    for(auto& transaction : m_transactions){
        if (transaction.getId() == m_transaction_to_be_updated.getId()) {
            transaction = m_transaction_to_be_updated; // Update the existing transaction
            break;
        }
    }

    // Buy-side data date isn't persisted, so report it against the recorded buy date
    FetchResult buyData;
    buyData.m_exchangeRate = { m_transaction_to_be_updated.getExchangeRateAtBuy(), m_transaction_to_be_updated.getBuyQDate() };
    buyData.m_inflationIndex = { m_transaction_to_be_updated.getInflationIndexAtBuy(), m_transaction_to_be_updated.getBuyQDate() };
    TransactionReporter::writeReport(m_transaction_to_be_updated, TransactionType::Close,
                                      buyData, m_data_to_be_updated, breakdown);

    m_data_to_be_updated = {}; // Reset after use

    emit databaseReady();
}

void TransactionManager::processPotentialTransaction() {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_transaction_to_be_updated.setExchangeRateAtSell(m_data_to_be_updated.m_exchangeRate.value);
    m_transaction_to_be_updated.setInflationIndexAtSell(m_data_to_be_updated.m_inflationIndex.value);
    TaxCalculationBreakdown breakdown = Calculator::calculateTaxBaseDetailed(m_transaction_to_be_updated);

    FetchResult buyData;
    buyData.m_exchangeRate = { m_transaction_to_be_updated.getExchangeRateAtBuy(), m_transaction_to_be_updated.getBuyQDate() };
    buyData.m_inflationIndex = { m_transaction_to_be_updated.getInflationIndexAtBuy(), m_transaction_to_be_updated.getBuyQDate() };
    TransactionReporter::writeReport(m_transaction_to_be_updated, TransactionType::Potential,
                                      buyData, m_data_to_be_updated, breakdown);

    m_data_to_be_updated = {}; // Reset after use
    emit potentialTaxBaseReady(breakdown.taxBase);
}

void TransactionManager::onEvdsDataFetched(const std::shared_ptr<QJsonObject> &data,
                                    const QString &seriesCode) {
    qDebug(logManager) << "EVDS data fetched for series:" << seriesCode;
    
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
        qWarning(logManager) << "Received data with no active transaction";
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
                        m_data_to_be_updated.m_exchangeRate.value = value;
                        m_exchangeRateReceived = true;
                        qInfo(logManager) << "Exchange rate received:" << value;
                        
                        QJsonValue exchangeDate = item.value("Tarih");
                        if (!exchangeDate.isNull()) {
                            if (exchangeDate.isString()) {
                                m_data_to_be_updated.m_exchangeRate.date = QDate::fromString(exchangeDate.toString(), "dd-MM-yyyy");
                            }
                        }
                        
                        qDebug(logManager) << "Exchange rate date received:" << m_data_to_be_updated.m_exchangeRate.date << "Value:" << value;
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
                        m_data_to_be_updated.m_inflationIndex.value = value;
                        m_inflationIndexReceived = true;
                        qInfo(logManager) << "Inflation index received:" << value;
                        
                        QJsonValue inflationDate = item.value("Tarih");
                        if (!inflationDate.isNull()) {
                            if (inflationDate.isString()) {
                                m_data_to_be_updated.m_inflationIndex.date = QDate::fromString(inflationDate.toString(), "yyyy-M");
                            }
                        }
                        
                        qDebug(logManager) << "Inflation index date received:" << m_data_to_be_updated.m_inflationIndex.date << "Value:" << value;
                        break;
                    }
                }
            }
        }
    }
    
    if (m_exchangeRateReceived && m_inflationIndexReceived) {
        qDebug(logManager) << "Both values received, processing transaction";
        
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
                qWarning(logManager) << "Unexpected transaction state during processing";
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
    qDebug(logManager) << "Transaction with ID" << id << "not found.";
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
    m_data_to_be_updated = {};
    
    qWarning(logManager) << "Data fetch failed:" << error;
    emit fetchFailed(error);
}