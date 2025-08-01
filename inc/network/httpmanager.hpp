#ifndef HTTPMANAGER_H
#define HTTPMANAGER_H

#include "getrequest.hpp"

#ifndef TOKEN
    #define TOKEN ""
#endif

class HttpManager : public GetRequest{
public:
    HttpManager() : GetRequest(this) {}

    void setKey(const QString &key) {
        this->key = key;
    }
    
    QString token{TOKEN};
    QString key;
};

#endif // HTTPMANAGER_H
