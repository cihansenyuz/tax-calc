#pragma once
#include <QSqlDatabase>
#include <QString>
#include <vector>

class Asset;

class AssetDatabase {
public:
    static AssetDatabase& getInstance(const QString& dbPath = "");
    ~AssetDatabase();

    bool isOpen() const;
    bool initAssetTable();
    bool idExists(int id);

    bool saveAsset(const Asset& asset);
    bool updateAsset(const Asset& asset);
    std::vector<Asset> loadAssets();

private:
    AssetDatabase(const QString& dbPath);
    AssetDatabase(const AssetDatabase&) = delete;
    AssetDatabase& operator=(const AssetDatabase&) = delete;
    QSqlDatabase m_db;
};
