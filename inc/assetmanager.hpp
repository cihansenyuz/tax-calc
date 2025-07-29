#pragma once

#include <vector>
#include <QObject>
#include <mutex>
#include <condition_variable>
#include "asset.hpp"
#include "database/assetdatabase.hpp"

class AssetManager : public QObject {
    Q_OBJECT
public:
    explicit AssetManager(QObject *parent = nullptr);
    ~AssetManager();

    void openTransaction(const Asset& asset);
    void closeTransaction(const Asset& asset);
    void updateAsset(Asset& asset);
    const std::vector<Asset>& getAssets() const { return m_assets; }
    void clear() { m_assets.clear(); }
    size_t size() const { return m_assets.size(); }
    bool empty() const { return m_assets.empty(); }

signals:
    void databaseReady();

private slots:
    void onEvdsDataFetched(const std::shared_ptr<QJsonObject> &data);

private:
    std::vector<Asset> m_assets;
    Asset m_asset_to_be_updated;
    std::pair<double, double> m_data_to_be_updated{0.0, 0.0}; // (USD, TUFE)
    std::mutex m_mutex;
    std::condition_variable m_condition;
    class HttpManager *m_http_manager;
    class EvdsFetcher *m_evds_fetcher;
    class AssetDatabase *m_asset_db;
    static constexpr const char* API_KEY = "HSzat3MFdF";
};
