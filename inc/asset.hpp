#pragma once
#include <string>
#include <QDate>

class Asset {
private:
    std::string m_symbol;
    std::string m_symbolName;
    QDate m_buyDate;
    double m_buyPrice;
    int m_quantity;
    QDate m_sellDate;
    double m_sellPrice;
    QString m_status;
    double m_inflationIndex;
    double m_exchangeRate;

public:
    Asset() = default;
    Asset(const std::string& symbol,
          const std::string& symbolName,
          QDate buyDate,
          double buyPrice,
          int quantity,
          QDate sellDate = QDate(),
          double sellPrice = 0.0,
          const QString& status = "Açık")
        : m_symbol(symbol),
          m_symbolName(symbolName),
          m_buyDate(buyDate),
          m_buyPrice(buyPrice),
          m_quantity(quantity),
          m_sellDate(sellDate),
          m_sellPrice(sellPrice),
          m_status(status) {
        m_inflationIndex = 0.0;
        m_exchangeRate = 0.0;
    }

    std::string getSymbol() const { return m_symbol; }
    std::string getSymbolName() const { return m_symbolName; }
    QString getBuyDate() const { return m_buyDate.toString("dd-MM-yyyy"); }
    double getBuyPrice() const { return m_buyPrice; }
    int getQuantity() const { return m_quantity; }
    QString getSellDate() const { return m_sellDate.isValid() ? m_sellDate.toString("dd-MM-yyyy") : ""; }
    double getSellPrice() const { return m_sellPrice; }
    QString getStatus() const { return m_status; }
    double getInflationIndex() const { return m_inflationIndex; }
    double getExchangeRate() const { return m_exchangeRate; }

    void setSymbol(const std::string& s) { m_symbol = s; }
    void setSymbolName(const std::string& n) { m_symbolName = n; }
    void setBuyPrice(double p) { m_buyPrice = p; }
    void setQuantity(int q) { m_quantity = q; }
    void setSellDate(const QDate& d) { m_sellDate = d; }
    void setSellPrice(double p) { m_sellPrice = p; }
    void setStatus(const QString& s) { m_status = s; }
    void setInflationIndex(double i) { m_inflationIndex = i; }
    void setExchangeRate(double r) { m_exchangeRate = r; }
};
