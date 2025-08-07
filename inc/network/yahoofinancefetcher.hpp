#pragma once

#include <QObject>
#include <QString>
#include <QList>
#include <QPair>
#include "httpmanager.hpp"

class YahooFinanceFetcher : public QObject {
    Q_OBJECT
public:
    explicit YahooFinanceFetcher(QObject *parent = nullptr);
    void fetchSymbols(const QString &query);

signals:
    void symbolsFetched(const QList<QPair<QString, QString>> &symbols); // symbol, shortname
    void fetchFailed(const QString &error);

private slots:
    void onJsonFetched(const std::shared_ptr<QJsonObject> &json);
    
private:
    HttpManager *http_manager_;
    static constexpr const char* apiEndPoint = "https://query1.finance.yahoo.com/v1/finance/search";
    static constexpr const char* quotesField = "quotesCount=10";
    static constexpr const char* newsCountField = "newsCount=0";
};
