#include "inc/database/assetdatabase.hpp"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

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
        "name TEXT NOT NULL,"
        "type TEXT,"
        "value REAL"
        ");";
    if (!query.exec(createTable)) {
        qWarning() << "Failed to create assets table:" << query.lastError().text();
        return false;
    }
    return true;
}
