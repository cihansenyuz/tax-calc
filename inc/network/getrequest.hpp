#pragma once

#include "networkcore.hpp"

class GetRequest : public NetworkCore
{
    Q_OBJECT
public:
    GetRequest(HttpManager *parent = nullptr);
    void fetchJsonData(const QString &api_query);

signals:
    void jsonFetched(const std::shared_ptr<QJsonObject> &fetched_data);

private slots:
    void onFetchJsonDataReplyReceived(QNetworkReply* reply);

private:
    QNetworkReply* getHttpReply(const QNetworkRequest &request) override;
    HttpManager *parent_;
};
