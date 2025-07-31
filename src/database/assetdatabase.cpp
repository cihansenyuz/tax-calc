#include "../../inc/database/assetdatabase.hpp"
#include "../../inc/asset.hpp"

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QString>

AssetDatabase& AssetDatabase::getInstance(const QString& dbPath) {
    static AssetDatabase instance(dbPath);
    return instance;
}

AssetDatabase::AssetDatabase(const QString& dbPath) {
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath);
    if (!m_db.open()) {
        qWarning() << "Failed to open database:" << m_db.lastError().text();
    }
}

AssetDatabase::~AssetDatabase() {
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool AssetDatabase::isOpen() const {
    return m_db.isOpen();
}

bool AssetDatabase::initAssetTable() {
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

bool AssetDatabase::saveAsset(const Asset& asset) {
    QSqlQuery query(m_db);
    query.prepare(
        "INSERT INTO assets (id, symbol, symbolName, buyDate, buyPrice, quantity, sellDate, sellPrice, status, inflationIndexAtBuy, exchangeRateAtBuy, inflationIndexAtSell, exchangeRateAtSell, tax, taxBase) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
    );

    query.addBindValue(asset.getId());
    query.addBindValue(QString::fromStdString(asset.getSymbol()));
    query.addBindValue(QString::fromStdString(asset.getSymbolName()));
    query.addBindValue(asset.getBuyDate());
    query.addBindValue(asset.getBuyPrice());
    query.addBindValue(asset.getQuantity());
    query.addBindValue(asset.getSellDate());
    query.addBindValue(asset.getSellPrice());
    query.addBindValue(asset.getStatus());
    query.addBindValue(asset.getInflationIndexAtBuy());
    query.addBindValue(asset.getExchangeRateAtBuy());
    query.addBindValue(asset.getInflationIndexAtSell());
    query.addBindValue(asset.getExchangeRateAtSell());
    query.addBindValue(asset.getTax());
    query.addBindValue(asset.getTaxBase());
    if (!query.exec()) {
        qWarning() << "Failed to insert asset:" << query.lastError().text();
        return false;
    }
    return true;
}

bool AssetDatabase::updateAsset(const Asset& asset) {
    QSqlQuery query(m_db);
    query.prepare(
        "UPDATE assets SET symbol=?, symbolName=?, buyDate=?, buyPrice=?, quantity=?, sellDate=?, sellPrice=?, status=?, inflationIndexAtBuy=?, exchangeRateAtBuy=?, inflationIndexAtSell=?, exchangeRateAtSell=?, tax=?, taxBase=? WHERE id=?"
    );
    query.addBindValue(QString::fromStdString(asset.getSymbol()));
    query.addBindValue(QString::fromStdString(asset.getSymbolName()));
    query.addBindValue(asset.getBuyDate());
    query.addBindValue(asset.getBuyPrice());
    query.addBindValue(asset.getQuantity());
    query.addBindValue(asset.getSellDate());
    query.addBindValue(asset.getSellPrice());
    query.addBindValue(asset.getStatus());
    query.addBindValue(asset.getInflationIndexAtBuy());
    query.addBindValue(asset.getExchangeRateAtBuy());
    query.addBindValue(asset.getInflationIndexAtSell());
    query.addBindValue(asset.getExchangeRateAtSell());
    query.addBindValue(asset.getTax());
    query.addBindValue(asset.getTaxBase());
    query.addBindValue(asset.getId());
    if (!query.exec()) {
        qWarning() << "Failed to update asset:" << query.lastError().text();
        return false;
    }
    return true;
}

bool AssetDatabase::deleteAsset(int id)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM assets WHERE id=?");
    query.addBindValue(id);
    if (!query.exec()) {
        qWarning() << "Failed to delete asset:" << query.lastError().text();
        return false;
    }
    return true;
}

std::vector<Asset> AssetDatabase::loadAssets()
{
    qDebug() << "Loading assets from database...";
    std::vector<Asset> assets;
    QSqlQuery query(m_db);

    if (!query.exec("SELECT id, symbol, symbolName, buyDate, buyPrice, quantity, sellDate, sellPrice, status, inflationIndexAtBuy, exchangeRateAtBuy, inflationIndexAtSell, exchangeRateAtSell, tax, taxBase FROM assets")) {
        qWarning() << "Failed to load assets:" << query.lastError().text();
        return assets;
    }
    while (query.next()) {
        Asset asset(
            query.value(0).toInt(), // id
            query.value(1).toString().toStdString(), // symbol
            query.value(2).toString().toStdString(), // symbolName
            QDate::fromString(query.value(3).toString(), "dd-MM-yyyy"), // buyDate
            query.value(4).toDouble(), // buyPrice
            query.value(5).toInt(), // quantity
            QDate::fromString(query.value(6).toString(), "dd-MM-yyyy"), // sellDate
            query.value(7).toDouble(), // sellPrice
            query.value(8).toString(), // status
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
        qDebug() << "Status:" << asset.getStatus();
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

bool AssetDatabase::idExists(int id) {
    QSqlQuery query(m_db);
    query.prepare("SELECT COUNT(*) FROM assets WHERE id=?");
    query.addBindValue(id);
    if (!query.exec() || !query.next()) {
        return false;
    }
    return query.value(0).toInt() > 0;
}