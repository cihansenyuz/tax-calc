#pragma once
#include <QSqlDatabase>
#include <QString>
#include <vector>

class Transaction;

class TransactionDatabase {
public:
    static TransactionDatabase& getInstance(const QString& dbPath = "");
    ~TransactionDatabase();

    bool isOpen() const;
    bool initAssetTable();
    bool idExists(int id);

    bool saveAsset(const Transaction& transaction);
    bool updateAsset(const Transaction& transaction);
    bool deleteAsset(int id);
    
    std::vector<Transaction> loadAssets();

private:
    TransactionDatabase(const QString& dbPath);
    TransactionDatabase(const TransactionDatabase&) = delete;
    TransactionDatabase& operator=(const TransactionDatabase&) = delete;
    QSqlDatabase m_db;
};
