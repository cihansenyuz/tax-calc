#include "../../inc/network/networkcore.h"

QJsonObject NetworkCore::ReadBody(){
    QByteArray raw_data = http_reply->readAll();
    QJsonDocument body_message_doc = QJsonDocument::fromJson(raw_data.data());
    return body_message_doc.object();
}
int NetworkCore::GetHttpStatusCode(){
    return http_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
}
