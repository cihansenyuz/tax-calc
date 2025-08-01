#include "../../inc/network/networkcore.hpp"

QJsonObject NetworkCore::readBody(QNetworkReply* reply) {
    QByteArray raw_data = reply->readAll();
    QJsonDocument body_message_doc = QJsonDocument::fromJson(raw_data.data());

    return body_message_doc.object();
}
int NetworkCore::getHttpStatusCode(QNetworkReply* reply) {
    return reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
}