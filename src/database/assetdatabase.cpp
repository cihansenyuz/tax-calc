#include "../../inc/database/assetdatabase.hpp"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QString>

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
        "inflationIndex REAL,"
        "exchangeRate REAL"
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
        "INSERT INTO assets (symbol, symbolName, buyDate, buyPrice, quantity, sellDate, sellPrice, status, inflationIndex, exchangeRate) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
    );

    query.addBindValue(QString::fromStdString(asset.getSymbol()));
    query.addBindValue(QString::fromStdString(asset.getSymbolName()));
    query.addBindValue(asset.getBuyDate());
    query.addBindValue(asset.getBuyPrice());
    query.addBindValue(asset.getQuantity());
    query.addBindValue(asset.getSellDate());
    query.addBindValue(asset.getSellPrice());
    query.addBindValue(asset.getStatus());
    query.addBindValue(asset.getInflationIndex());
    query.addBindValue(asset.getExchangeRate());
    if (!query.exec()) {
        qWarning() << "Failed to insert asset:" << query.lastError().text();
        return false;
    }
    return true;
}

bool AssetDatabase::updateAsset(const Asset& asset) {
    QSqlQuery query(m_db);
    // Assuming symbol is unique for update
    query.prepare(
        "UPDATE assets SET symbolName=?, buyDate=?, buyPrice=?, quantity=?, sellDate=?, sellPrice=?, status=?, inflationIndex=?, exchangeRate=? WHERE symbol=?"
    );
    query.addBindValue(QString::fromStdString(asset.getSymbolName()));
    query.addBindValue(asset.getBuyDate());
    query.addBindValue(asset.getBuyPrice());
    query.addBindValue(asset.getQuantity());
    query.addBindValue(asset.getSellDate());
    query.addBindValue(asset.getSellPrice());
    query.addBindValue(asset.getStatus());
    query.addBindValue(asset.getInflationIndex());
    query.addBindValue(asset.getExchangeRate());
    query.addBindValue(QString::fromStdString(asset.getSymbol()));
    if (!query.exec()) {
        qWarning() << "Failed to update asset:" << query.lastError().text();
        return false;
    }
    return true;
}

std::vector<Asset> AssetDatabase::loadAssets() {
    std::vector<Asset> assets;
    QSqlQuery query(m_db);

    if (!query.exec("SELECT symbol, symbolName, buyDate, buyPrice, quantity, sellDate, sellPrice, status, inflationIndex, exchangeRate FROM assets")) {
        qWarning() << "Failed to load assets:" << query.lastError().text();
        return assets;
    }
    while (query.next()) {
        Asset asset(
            query.value(0).toString().toStdString(),
            query.value(1).toString().toStdString(),
            QDate::fromString(query.value(2).toString(), Qt::ISODate),
            query.value(3).toDouble(),
            query.value(4).toInt(),
            QDate::fromString(query.value(5).toString(), Qt::ISODate),
            query.value(6).toDouble(),
            query.value(7).toString()
        );
        asset.setInflationIndex(query.value(8).toDouble());
        asset.setExchangeRate(query.value(9).toDouble());
        assets.push_back(asset);
    }
    return assets;
}
