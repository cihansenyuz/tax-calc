#include "../../inc/network/getrequest.hpp"
#include "../../inc/network/httpmanager.hpp"
#include "../../inc/logger.hpp"

GetRequest::GetRequest(HttpManager *parent)
    : parent_(parent) {}

void GetRequest::onFetchJsonDataReplyReceived(QNetworkReply* reply) {
    qDebug(logNetwork) << "#### on fetch(get) reply ####";
    
    if (!reply) {
        qDebug(logNetwork) << "Error: Reply is null";
        qDebug(logNetwork) << "#########################################\n";
        return;
    }
    
    if(reply->error() == QNetworkReply::NoError){
        QByteArray replyData = reply->readAll();
        QJsonDocument replyDocument = QJsonDocument::fromJson(replyData.data());
        std::shared_ptr<QJsonObject> fetched_data;

        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if(statusCode == 200){
            fetched_data = std::make_shared<QJsonObject>(replyDocument.object());
            emit jsonFetched(fetched_data);
            qInfo(logNetwork) << "json data fetched successfully";
        }
        else if(statusCode == 401 || statusCode == 403)
            qWarning(logNetwork) << "fetch failed, unauthorized attempt";
        else if(replyDocument.isNull())
            qWarning(logNetwork) << "JSON array is null";
        else if(!replyDocument.isArray())
            qWarning(logNetwork) << "JSON is not an array.";
    }
    else
        qWarning(logNetwork) << "fetch error: " << reply->error();
    qDebug(logNetwork) << "#########################################\n";
}

void GetRequest::fetchJsonData(const QString &api_query) {
    sendHttpRequest(api_query, parent_->key, this, &GetRequest::onFetchJsonDataReplyReceived);
}

QNetworkReply* GetRequest::getHttpReply(const QNetworkRequest &request) {
    qInfo(logNetwork) << "get request done";
    return http_access_manager.get(request);
}
