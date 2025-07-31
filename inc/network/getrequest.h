#ifndef GETREQUEST_H
#define GETREQUEST_H

#include "networkcore.h"

class GetRequest : public NetworkCore
{
    Q_OBJECT
public:
    GetRequest(HttpManager *parent = nullptr);
    void FetchJsonData(const QString &api_query);

signals:
    void JsonFetched(const std::shared_ptr<QJsonObject> &fetched_data);

private slots:
    void OnFetchJsonDataReplyRecieved(QNetworkReply* reply);

private:
    QNetworkReply* GetHttpReply(const QNetworkRequest &request) override;
    HttpManager *parent_;
};

#endif // GETREQUEST_H
