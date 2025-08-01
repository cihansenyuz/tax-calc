#include "../../inc/database/transactiondatabase.hpp"
#include "../../inc/transaction.hpp"

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QString>

TransactionDatabase& TransactionDatabase::getInstance(const QString& dbPath) {
    static TransactionDatabase instance(dbPath);
    return instance;
}

TransactionDatabase::TransactionDatabase(const QString& dbPath) {
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath);
    if (!m_db.open()) {
        qWarning() << "Failed to open database:" << m_db.lastError().text();
    }
}

TransactionDatabase::~TransactionDatabase() {
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool TransactionDatabase::isOpen() const {
    return m_db.isOpen();
}

bool TransactionDatabase::initAssetTable() {
    QSqlQuery query(m_db);
    QString createTable =
        "CREATE TABLE IF NOT EXISTS assets ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "symbol TEXT NOT NULL,"
        "symbolName TEXT,"
        "buyDate TEXT,"
        "buyPrice REAL,"
        "quantity INTEGER,"
        "sellDate TEXT,"
        "sellPrice REAL,"
        "status TEXT,"
        "inflationIndexAtBuy REAL,"
        "exchangeRateAtBuy REAL,"
        "inflationIndexAtSell REAL,"
        "exchangeRateAtSell REAL,"
        "tax REAL,"
        "taxBase REAL"
        ");";
    if (!query.exec(createTable)) {
        qWarning() << "Failed to create assets table:" << query.lastError().text();
        return false;
    }
    return true;
}

bool TransactionDatabase::saveAsset(const Transaction& transaction) {
    QSqlQuery query(m_db);
    query.prepare(
        "INSERT INTO assets (id, symbol, symbolName, buyDate, buyPrice, quantity, sellDate, sellPrice, status, inflationIndexAtBuy, exchangeRateAtBuy, inflationIndexAtSell, exchangeRateAtSell, tax, taxBase) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
    );

    query.addBindValue(transaction.getId());
    query.addBindValue(QString::fromStdString(transaction.getSymbol()));
    query.addBindValue(QString::fromStdString(transaction.getSymbolName()));
    query.addBindValue(transaction.getBuyDate());
    query.addBindValue(transaction.getBuyPrice());
    query.addBindValue(transaction.getQuantity());
    query.addBindValue(transaction.getSellDate());
    query.addBindValue(transaction.getSellPrice());
    query.addBindValue(Transaction::statusToString(transaction.getStatus()));
    query.addBindValue(transaction.getInflationIndexAtBuy());
    query.addBindValue(transaction.getExchangeRateAtBuy());
    query.addBindValue(transaction.getInflationIndexAtSell());
    query.addBindValue(transaction.getExchangeRateAtSell());
    query.addBindValue(transaction.getTax());
    query.addBindValue(transaction.getTaxBase());
    if (!query.exec()) {
        qWarning() << "Failed to insert asset:" << query.lastError().text();
        return false;
    }
    return true;
}

bool TransactionDatabase::updateAsset(const Transaction& transaction) {
    QSqlQuery query(m_db);
    query.prepare(
        "UPDATE assets SET symbol=?, symbolName=?, buyDate=?, buyPrice=?, quantity=?, sellDate=?, sellPrice=?, status=?, inflationIndexAtBuy=?, exchangeRateAtBuy=?, inflationIndexAtSell=?, exchangeRateAtSell=?, tax=?, taxBase=? WHERE id=?"
    );
    query.addBindValue(QString::fromStdString(transaction.getSymbol()));
    query.addBindValue(QString::fromStdString(transaction.getSymbolName()));
    query.addBindValue(transaction.getBuyDate());
    query.addBindValue(transaction.getBuyPrice());
    query.addBindValue(transaction.getQuantity());
    query.addBindValue(transaction.getSellDate());
    query.addBindValue(transaction.getSellPrice());
    query.addBindValue(Transaction::statusToString(transaction.getStatus()));
    query.addBindValue(transaction.getInflationIndexAtBuy());
    query.addBindValue(transaction.getExchangeRateAtBuy());
    query.addBindValue(transaction.getInflationIndexAtSell());
    query.addBindValue(transaction.getExchangeRateAtSell());
    query.addBindValue(transaction.getTax());
    query.addBindValue(transaction.getTaxBase());
    query.addBindValue(transaction.getId());
    if (!query.exec()) {
        qWarning() << "Failed to update asset:" << query.lastError().text();
        return false;
    }
    return true;
}

bool TransactionDatabase::deleteAsset(int id) {
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM assets WHERE id=?");
    query.addBindValue(id);

    if (!query.exec()) {
        qWarning() << "Failed to delete asset:" << query.lastError().text();
        return false;
    }

    return true;
}

std::vector<Transaction> TransactionDatabase::loadAssets()
{
    qDebug() << "Loading assets from database...";
    std::vector<Transaction> assets;
    QSqlQuery query(m_db);

    if (!query.exec("SELECT id, symbol, symbolName, buyDate, buyPrice, quantity, sellDate, sellPrice, status, inflationIndexAtBuy, exchangeRateAtBuy, inflationIndexAtSell, exchangeRateAtSell, tax, taxBase FROM assets")) {
        qWarning() << "Failed to load assets:" << query.lastError().text();
        return assets;
    }

    while (query.next()) {
        Transaction asset(
            query.value(0).toInt(), // id
            query.value(1).toString().toStdString(), // symbol
            query.value(2).toString().toStdString(), // symbolName
            QDate::fromString(query.value(3).toString(), "dd-MM-yyyy"), // buyDate
            query.value(4).toDouble(), // buyPrice
            query.value(5).toInt(), // quantity
            QDate::fromString(query.value(6).toString(), "dd-MM-yyyy"), // sellDate
            query.value(7).toDouble(), // sellPrice
            Transaction::stringToStatus(query.value(8).toString()), // status
            query.value(9).toDouble(), // inflationIndexAtBuy
            query.value(10).toDouble(), // exchangeRateAtBuy
            query.value(11).toDouble(), // inflationIndexAtSell
            query.value(12).toDouble(), // exchangeRateAtSell
            query.value(13).toDouble(), // tax
            query.value(14).toDouble()  // taxBase
        );
        
        // Print each property of the loaded asset
        qDebug() << "=== Loaded Asset Properties ===";
        qDebug() << "ID:" << asset.getId();
        qDebug() << "Symbol:" << QString::fromStdString(asset.getSymbol());
        qDebug() << "Symbol Name:" << QString::fromStdString(asset.getSymbolName());
        qDebug() << "Buy Date:" << asset.getBuyDate();
        qDebug() << "Buy Price:" << asset.getBuyPrice();
        qDebug() << "Quantity:" << asset.getQuantity();
        qDebug() << "Sell Date:" << asset.getSellDate();
        qDebug() << "Sell Price:" << asset.getSellPrice();
        qDebug() << "Status:" << Transaction::statusToString(asset.getStatus());
        qDebug() << "Inflation Index At Buy:" << asset.getInflationIndexAtBuy();
        qDebug() << "Exchange Rate At Buy:" << asset.getExchangeRateAtBuy();
        qDebug() << "Inflation Index At Sell:" << asset.getInflationIndexAtSell();
        qDebug() << "Exchange Rate At Sell:" << asset.getExchangeRateAtSell();
        qDebug() << "Tax Base:" << asset.getTaxBase();
        qDebug() << "Tax:" << asset.getTax();
        qDebug() << "===============================";
        
        assets.push_back(asset);
    }
    qDebug() << "Successfully loaded" << assets.size() << "assets from database.";
    
    return assets;
}

bool TransactionDatabase::idExists(int id) {
    QSqlQuery query(m_db);
    query.prepare("SELECT COUNT(*) FROM assets WHERE id=?");
    query.addBindValue(id);
    if (!query.exec() || !query.next()) {
        return false;
    }
    return query.value(0).toInt() > 0;
}