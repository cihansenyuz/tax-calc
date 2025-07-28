#ifndef NETWORKCORE_H
#define NETWORKCORE_H

#include <QObject>
#include <QtNetwork>

#define HOST "https://evds2.tcmb.gov.tr/service/evds/series="

class HttpManager;

class NetworkCore : public QObject
{
    Q_OBJECT
public:
    NetworkCore() = default;
    virtual QNetworkReply* GetHttpReply(const QNetworkRequest &request) = 0;
    template<typename T>
    void SendHttpRequest(const QString &api_query,
                         const QString &key,
                         T* requester_object,
                         void (T::*slot_function)()){
        QUrl http_url(QString(HOST) + api_query);
        QNetworkRequest http_request(http_url);
        http_request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        if(key.size())
            http_request.setRawHeader("key", key.toUtf8());

        qDebug() << "#### on http request, http headers ####";
        QList<QByteArray> headerList = http_request.rawHeaderList();
        for (const QByteArray &header : headerList) {
            qDebug() << header << ": " << http_request.rawHeader(header);
        }
        qDebug() << "#########################################";

        http_reply = GetHttpReply(http_request);
        connect(http_reply, &QNetworkReply::finished,
                requester_object, slot_function,
                Qt::SingleShotConnection);
    }
    QJsonObject ReadBody();
    int GetHttpStatusCode();

    QNetworkAccessManager http_access_manager;
    QJsonDocument http_body_data;
    QNetworkReply *http_reply;
};

#endif // NETWORKCORE_H
