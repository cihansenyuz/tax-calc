#include "../../inc/network/getrequest.hpp"
#include "../../inc/network/httpmanager.hpp"

GetRequest::GetRequest(HttpManager *parent)
    : parent_(parent) {}

void GetRequest::onFetchJsonDataReplyReceived(QNetworkReply* reply) {
    qDebug() << "#### on fetch(get) reply ####";
    
    if (!reply) {
        qDebug() << "Error: Reply is null";
        qDebug() << "#########################################\n";
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
            qDebug() << "json data fetched successfully";
        }
        else if(statusCode == 401 || statusCode == 403)
            qDebug() << "fetch failed, unauthorized attempt";
        else if(replyDocument.isNull())
            qDebug() << "JSON array is null";
        else if(!replyDocument.isArray())
            qDebug() << "JSON is not an array.";
    }
    else
        qDebug() << "fetch error: " << reply->error();
    qDebug() << "#########################################\n";
}

void GetRequest::fetchJsonData(const QString &api_query) {
    sendHttpRequest(api_query, parent_->key, this, &GetRequest::onFetchJsonDataReplyReceived);
}

QNetworkReply* GetRequest::getHttpReply(const QNetworkRequest &request) {
    qDebug() << "get request done";
    return http_access_manager.get(request, http_body_data.toJson());
}
