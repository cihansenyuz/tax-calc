#pragma once

#include <vector>
#include <QObject>
#include "asset.hpp"

class AssetManager : public QObject {
    Q_OBJECT
public:
    explicit AssetManager(QObject *parent = nullptr);
    ~AssetManager();

    void addAsset(const Asset& asset);
    void updateAsset(Asset& asset);
    const std::vector<Asset>& getAssets() const { return m_assets; }
    void clear() { m_assets.clear(); }
    size_t size() const { return m_assets.size(); }
    bool empty() const { return m_assets.empty(); }

signals:
    void assetDataFetched(const std::shared_ptr<QJsonObject> &data);

private slots:
    void onEvdsDataFetched(const std::shared_ptr<QJsonObject> &data);

private:
    std::vector<Asset> m_assets;
    Asset m_asset_to_be_updated;
    class HttpManager *m_http_manager;
    class EvdsFetcher *m_evds_fetcher;
    static constexpr const char* API_KEY = "HSzat3MFdF";
};
