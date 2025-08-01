#pragma once

#include <vector>
#include <QObject>
#include <mutex>
#include <condition_variable>
#include "transaction.hpp"
#include "database/transactiondatabase.hpp"

class TransactionManager : public QObject {
    Q_OBJECT
public:
    enum class TransactionType {
        None,
        Open,
        Close,
        Potential
    };

    enum class DataType {
        None,
        ExchangeRate,
        InflationIndex
    };

    explicit TransactionManager(QObject *parent = nullptr);
    ~TransactionManager();

    void openTransaction(const Transaction& transaction);
    void closeTransaction(const Transaction& transaction);
    void potentialTransaction(const Transaction& transaction);
    void updateAsset(Transaction& transaction);
    Transaction findTransactionById(int id);
    void removeTransaction(int id);
    const std::vector<Transaction>& getTransactions() const { return m_transactions; }
    void clear() { m_transactions.clear(); }
    size_t size() const { return m_transactions.size(); }
    bool empty() const { return m_transactions.empty(); }

signals:
    void databaseReady();
    void potentialTaxBaseReady(double potentialTaxBase);
    void fetchFailed(const QString &error);

private slots:
    void onEvdsDataFetched(const std::shared_ptr<QJsonObject> &data,
                            const QString &seriesCode);
    void onFetchFailed(const QString &error);
    
private:
    std::vector<Transaction> m_transactions;
    Transaction m_transaction_to_be_updated;
    std::pair<double, double> m_data_to_be_updated{0.0, 0.0}; // (USD, TUFE)
    std::mutex m_mutex;
    TransactionType m_currentTransactionType;
    bool m_exchangeRateReceived{false};
    bool m_inflationIndexReceived{false};
    class HttpManager *m_http_manager;
    class EvdsFetcher *m_evds_fetcher;
    class TransactionDatabase *m_asset_db;

    void processOpenTransaction();
    void processCloseTransaction();
    void processPotentialTransaction();
};
