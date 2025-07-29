#pragma once
#include <string>
#include <QDate>

#include "../inc/database/assetdatabase.hpp"

class Asset {
private:
    int m_id = -1;
    std::string m_symbol;
    std::string m_symbolName;
    QDate m_buyDate;
    double m_buyPrice;
    int m_quantity;
    QDate m_sellDate;
    double m_sellPrice;
    QString m_status;
    double m_inflationIndexAtBuy;
    double m_exchangeRateAtBuy;
    double m_inflationIndexAtSell;
    double m_exchangeRateAtSell;

public:
    Asset() = default;
    Asset(int id,
          const std::string& symbol,
          const std::string& symbolName,
          QDate buyDate,
          double buyPrice,
          int quantity,
          QDate sellDate = QDate(),
          double sellPrice = 0.0,
          const QString& status = "Açık",
          double inflationIndexAtBuy = 0.0,
          double exchangeRateAtBuy = 0.0,
          double inflationIndexAtSell = 0.0,
          double exchangeRateAtSell = 0.0)
        : m_id(id),
          m_symbol(symbol),
          m_symbolName(symbolName),
          m_buyDate(buyDate),
          m_buyPrice(buyPrice),
          m_quantity(quantity),
          m_sellDate(sellDate),
          m_sellPrice(sellPrice),
          m_status(status),
          m_inflationIndexAtBuy(inflationIndexAtBuy),
          m_exchangeRateAtBuy(exchangeRateAtBuy),
          m_inflationIndexAtSell(inflationIndexAtSell),
          m_exchangeRateAtSell(exchangeRateAtSell) {}

    static Asset createWithUniqueId(
           const std::string& symbol,
           const std::string& symbolName,
           QDate buyDate,
           double buyPrice,
           int quantity) {
        int id;
        do {
            id = rand();
        } while (AssetDatabase::getInstance().idExists(id));
        return Asset(id, symbol, symbolName, buyDate, buyPrice, quantity);
    }

    int getId() const { return m_id; }
    void setId(int id) { m_id = id; }
    std::string getSymbol() const { return m_symbol; }
    std::string getSymbolName() const { return m_symbolName; }
    QString getBuyDate() const { return m_buyDate.toString("dd-MM-yyyy"); }
    double getBuyPrice() const { return m_buyPrice; }
    int getQuantity() const { return m_quantity; }
    QString getSellDate() const { return m_sellDate.isValid() ? m_sellDate.toString("dd-MM-yyyy") : ""; }
    double getSellPrice() const { return m_sellPrice; }
    QString getStatus() const { return m_status; }
    double getInflationIndexAtBuy() const { return m_inflationIndexAtBuy; }
    double getExchangeRateAtBuy() const { return m_exchangeRateAtBuy; }
    double getInflationIndexAtSell() const { return m_inflationIndexAtSell; }
    double getExchangeRateAtSell() const { return m_exchangeRateAtSell; }

    void setSymbol(const std::string& s) { m_symbol = s; }
    void setSymbolName(const std::string& n) { m_symbolName = n; }
    void setBuyPrice(double p) { m_buyPrice = p; }
    void setQuantity(int q) { m_quantity = q; }
    void setSellDate(const QDate& d) { m_sellDate = d; }
    void setSellPrice(double p) { m_sellPrice = p; }
    void setStatus(const QString& s) { m_status = s; }
    void setInflationIndexAtBuy(double i) { m_inflationIndexAtBuy = i; }
    void setExchangeRateAtBuy(double r) { m_exchangeRateAtBuy = r; }
    void setInflationIndexAtSell(double i) { m_inflationIndexAtSell = i; }
    void setExchangeRateAtSell(double r) { m_exchangeRateAtSell = r; }
};
