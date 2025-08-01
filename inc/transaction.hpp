#pragma once
#include <string>
#include <QDate>

#include "../inc/database/transactiondatabase.hpp"

class Transaction {
public:
    enum class Status {
        Unknown,
        Open,
        Closed
    };
    static QString statusToString(Status status) {
        switch (status) {
            case Status::Open:   return "Açık";
            case Status::Closed: return "Kapalı";
            default:            return "Bilinmiyor";
        }
    };
    static Status stringToStatus(const QString& statusStr) {
        if (statusStr == "Açık") return Status::Open;
        if (statusStr == "Kapalı") return Status::Closed;
        return Status::Unknown;
    }

    Transaction() = default;
    Transaction(int id,
          const std::string& symbol,
          const std::string& symbolName,
          QDate buyDate,
          double buyPrice,
          int quantity,
          QDate sellDate = QDate(),
          double sellPrice = 0.0,
          Status status = Status::Open,
          double inflationIndexAtBuy = 0.0,
          double exchangeRateAtBuy = 0.0,
          double inflationIndexAtSell = 0.0,
          double exchangeRateAtSell = 0.0,
          double tax = 0.0,
          double taxBase = 0.0)
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
          m_exchangeRateAtSell(exchangeRateAtSell),
          m_tax(tax),
          m_taxBase(taxBase) {}

    static Transaction createWithUniqueId(
           const std::string& symbol,
           const std::string& symbolName,
           QDate buyDate,
           double buyPrice,
           int quantity) {
        int id;
        do {
            id = rand();
        } while (TransactionDatabase::getInstance().idExists(id));
        return Transaction(id, symbol, symbolName, buyDate, buyPrice, quantity);
    }

    int getId() const { return m_id; }
    std::string getSymbol() const { return m_symbol; }
    std::string getSymbolName() const { return m_symbolName; }
    QString getBuyDate() const { return m_buyDate.toString("dd-MM-yyyy"); }
    QDate getBuyQDate() const { return m_buyDate; }
    double getBuyPrice() const { return m_buyPrice; }
    int getQuantity() const { return m_quantity; }
    QString getSellDate() const { return m_sellDate.isValid() ? m_sellDate.toString("dd-MM-yyyy") : ""; }
    QDate getSellQDate() const { return m_sellDate; }
    double getSellPrice() const { return m_sellPrice; }
    Status getStatus() const { return m_status; }
    double getInflationIndexAtBuy() const { return m_inflationIndexAtBuy; }
    double getExchangeRateAtBuy() const { return m_exchangeRateAtBuy; }
    double getInflationIndexAtSell() const { return m_inflationIndexAtSell; }
    double getExchangeRateAtSell() const { return m_exchangeRateAtSell; }
    double getTax() const { return m_tax; }
    double getTaxBase() const { return m_taxBase; }
    
    void setId(int id) { m_id = id; }
    void setSymbol(const std::string& s) { m_symbol = s; }
    void setSymbolName(const std::string& n) { m_symbolName = n; }
    void setBuyPrice(double p) { m_buyPrice = p; }
    void setQuantity(int q) { m_quantity = q; }
    void setSellDate(const QDate& d) { m_sellDate = d; }
    void setSellPrice(double p) { m_sellPrice = p; }
    void setStatus(Status s) { m_status = s; }
    void setInflationIndexAtBuy(double i) { m_inflationIndexAtBuy = i; }
    void setExchangeRateAtBuy(double r) { m_exchangeRateAtBuy = r; }
    void setInflationIndexAtSell(double i) { m_inflationIndexAtSell = i; }
    void setExchangeRateAtSell(double r) { m_exchangeRateAtSell = r; }
    void setTax(double t) { m_tax = t; }
    void setTaxBase(double tb) { m_taxBase = tb; }

private:
    int m_id = -1;
    std::string m_symbol;
    std::string m_symbolName;
    QDate m_buyDate;
    double m_buyPrice;
    int m_quantity;
    QDate m_sellDate;
    double m_sellPrice;
    Status m_status = Status::Open;
    double m_inflationIndexAtBuy;
    double m_exchangeRateAtBuy;
    double m_inflationIndexAtSell;
    double m_exchangeRateAtSell;
    double m_tax;
    double m_taxBase;
};