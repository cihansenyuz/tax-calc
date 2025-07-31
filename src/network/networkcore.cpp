#include "../../inc/network/networkcore.h"

QJsonObject NetworkCore::ReadBody(QNetworkReply* reply){
    QByteArray raw_data = reply->readAll();
    QJsonDocument body_message_doc = QJsonDocument::fromJson(raw_data.data());
    return body_message_doc.object();
}
int NetworkCore::GetHttpStatusCode(QNetworkReply* reply){
    return reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
}
