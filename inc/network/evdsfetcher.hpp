#pragma once

#include "getrequest.hpp"
#include "httpmanager.hpp"
#include <QObject>
#include <QString>
#include <QDate>
#include <memory>

class EvdsFetcher : public QObject {
    Q_OBJECT
public:
    explicit EvdsFetcher(HttpManager *manager, QObject *parent = nullptr);
    void fetchExchangeRate(QDate date);
    void fetchInflationIndex(QDate date);

    static constexpr const char* SERIES_USD = "TP.DK.USD.A";
    static constexpr const char* SERIES_INFLATION = "TP.TUFE1YI.T1";
    static constexpr const char* API_KEY = "HSzat3MFdF";

signals:
    void evdsDataFetched(const std::shared_ptr<QJsonObject> &data,
                        const QString &seriesCode);
    void fetchFailed(const QString &error);

private slots:
    void onJsonFetched(const std::shared_ptr<QJsonObject> &data);

private:
    HttpManager *http_manager_;
    GetRequest *getRequest_;
};
