#pragma once
#include <QSqlDatabase>
#include <QString>

class AssetDatabase {
public:
    AssetDatabase(const QString& dbPath);
    ~AssetDatabase();

    bool isOpen() const;
    bool initAssetTable();

private:
    QSqlDatabase m_db;
};
