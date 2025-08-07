#pragma once

#include "getrequest.hpp"

#ifndef TOKEN
    #define TOKEN ""
#endif

class HttpManager : public GetRequest {
public:
    static HttpManager* getInstance() {
        static HttpManager instance;
        return &instance;
    }

    void setKey(const QString &key) {
        this->key = key;
    }

    QString token{TOKEN};
    QString key;

private:
    HttpManager() : GetRequest(this) {}
    HttpManager(const HttpManager&) = delete;
    HttpManager& operator=(const HttpManager&) = delete;
};
