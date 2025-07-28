#ifndef EVDSFETCHER_H
#define EVDSFETCHER_H

#include "getrequest.h"
#include "httpmanager.h"
#include <QObject>
#include <QString>
#include <memory>

class EvdsFetcher : public QObject {
    Q_OBJECT
public:
    explicit EvdsFetcher(HttpManager *manager, QObject *parent = nullptr);
    void fetch(const QString &series, const QString &startDate, const QString &endDate);

    static constexpr const char* USD = "TP.DK.USD.A";

signals:
    void evdsDataFetched(const std::shared_ptr<QJsonObject> &data);
    void fetchFailed(const QString &error);

private slots:
    void onJsonFetched(const std::shared_ptr<QJsonObject> &data);

private:
    HttpManager *http_manager_;
    GetRequest *getRequest_;
};

#endif // EVDSFETCHER_H
