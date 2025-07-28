#include "../../inc/network/getrequest.h"
#include "../../inc/network/httpmanager.h"

GetRequest::GetRequest(HttpManager *parent)
    : parent_(parent) {}

void GetRequest::OnFetchJsonDataReplyRecieved(){
    qDebug() << "#### on fetch(get) reply ####";
    if(http_reply->error() == QNetworkReply::NoError){
        QByteArray reply = http_reply->readAll();
        http_body_data = QJsonDocument::fromJson(reply.data());
        std::shared_ptr<QJsonObject> fetched_data;

        if(GetHttpStatusCode() == 200){
            fetched_data = std::make_shared<QJsonObject>(http_body_data.object());
            emit JsonFetched(fetched_data);
            qDebug() << "json data fetched successfully";
        }
        else if(GetHttpStatusCode() == 401 || GetHttpStatusCode() == 403)
            qDebug() << "fetch failed, unauthorized attempt";
        else if(http_body_data.isNull())
            qDebug() << "JSON array is null";
        else if(!http_body_data.isArray())
            qDebug() << "JSON is not an array.";
    }
    else
        qDebug() << "fetch error: " << http_reply->error();
    qDebug() << "#########################################\n";
}

void GetRequest::FetchJsonData(){
    
    SendHttpRequest("TP.DK.USD.A&startDate=01-01-2025&endDate=01-20-2025&type=json", parent_->key, this, &GetRequest::OnFetchJsonDataReplyRecieved);
}

QNetworkReply* GetRequest::GetHttpReply(const QNetworkRequest &request){
    qDebug() << "get request done";
    return http_access_manager.get(request, http_body_data.toJson());
}
