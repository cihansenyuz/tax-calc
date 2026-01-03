#pragma once

#include "../logger.hpp"
#include <QObject>
#include <QtNetwork>

class HttpManager;

class NetworkCore : public QObject {
    Q_OBJECT
public:
    NetworkCore() = default;
    virtual QNetworkReply* getHttpReply(const QNetworkRequest &request) = 0;
    template<typename T>
    void sendHttpRequest(const QString &full_url,
                         const QString &key,
                         T* requester_object,
                         void (T::*slot_function)(QNetworkReply*)){
        QNetworkRequest http_request(QUrl{full_url});
        http_request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        if(key.size())
            http_request.setRawHeader("key", key.toUtf8());

        qDebug(logNetwork) << "#### on http request, http headers ####";
        QList<QByteArray> headerList = http_request.rawHeaderList();
        for (const QByteArray &header : headerList) {
            qDebug(logNetwork) << header << ": " << http_request.rawHeader(header);
        }
        qDebug(logNetwork) << "#########################################";

        QNetworkReply* reply = getHttpReply(http_request);
        
        // Use lambda to capture the reply and pass it to the slot
        connect(reply, &QNetworkReply::finished, [requester_object, slot_function, reply]() {
            (requester_object->*slot_function)(reply);
        });
        
        // Auto-delete the reply when finished to prevent memory leaks
        connect(reply, &QNetworkReply::finished,
                reply, &QNetworkReply::deleteLater,
                Qt::SingleShotConnection);
    }
    QJsonObject readBody(QNetworkReply* reply);
    int getHttpStatusCode(QNetworkReply* reply);

    QNetworkAccessManager http_access_manager;
    QJsonDocument http_body_data;
};
