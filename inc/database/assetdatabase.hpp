#pragma once
#include <QSqlDatabase>
#include <QString>
#include <vector>
#include "inc/asset.hpp"

class AssetDatabase {
public:
    AssetDatabase(const QString& dbPath);
    ~AssetDatabase();

    bool isOpen() const;
    bool initAssetTable();

    bool saveAsset(const Asset& asset);
    bool updateAsset(const Asset& asset);
    std::vector<Asset> loadAssets();

private:
    QSqlDatabase m_db;
};
