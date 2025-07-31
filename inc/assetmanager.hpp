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
    enum class TransactionType {
        None,
        Open,
        Close,
        Potential
    };

    enum class DataType {
        None,
        ExchangeRate,
        InflationIndex
    };

    explicit AssetManager(QObject *parent = nullptr);
    ~AssetManager();

    void openTransaction(const Asset& asset);
    void closeTransaction(const Asset& asset);
    void potentialTransaction(const Asset& asset);
    void updateAsset(Asset& asset);
    Asset findAssetById(int id);
    void removeAsset(int id);
    const std::vector<Asset>& getAssets() const { return m_assets; }
    void clear() { m_assets.clear(); }
    size_t size() const { return m_assets.size(); }
    bool empty() const { return m_assets.empty(); }

signals:
    void databaseReady();
    void potentialTaxBaseReady(double potentialTaxBase);

private slots:
    void onEvdsDataFetched(const std::shared_ptr<QJsonObject> &data,
                            const QString &seriesCode);

private:
    std::vector<Asset> m_assets;
    Asset m_asset_to_be_updated;
    std::pair<double, double> m_data_to_be_updated{0.0, 0.0}; // (USD, TUFE)
    std::mutex m_mutex;
    TransactionType m_currentTransactionType;
    bool m_exchangeRateReceived{false};
    bool m_inflationIndexReceived{false};
    class HttpManager *m_http_manager;
    class EvdsFetcher *m_evds_fetcher;
    class AssetDatabase *m_asset_db;

    void processOpenTransaction();
    void processCloseTransaction();
    void processPotentialTransaction();
};
