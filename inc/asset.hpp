#pragma once
#include <string>
#include <QDate>

class Asset {
private:
    std::string m_symbol;
    std::string m_symbolName;
    QDate m_buyDate;
    double m_buyPrice;
    double m_inflationIndex;
    double m_exchangeRate;

public:
    Asset(  const std::string& symbol,
            const std::string& symbolName,
            QDate buyDate,
            double buyPrice)
        : m_symbol(symbol),
          m_symbolName(symbolName),
          m_buyDate(buyDate),
          m_buyPrice(buyPrice) {
                m_inflationIndex = 0.0;
                m_exchangeRate = 0.0;
          }

    std::string getSymbol() const { return m_symbol; }
    std::string getSymbolName() const { return m_symbolName; }
    QString getBuyDate() const { return m_buyDate.toString("dd-MM-yyyy"); }
    double getBuyPrice() const { return m_buyPrice; }
    double getInflationIndex() const { return m_inflationIndex; }
    double getExchangeRate() const { return m_exchangeRate; }

    void setSymbol(const std::string& s) { m_symbol = s; }
    void setSymbolName(const std::string& n) { m_symbolName = n; }
    void setBuyPrice(double p) { m_buyPrice = p; }
    void setInflationIndex(double i) { m_inflationIndex = i; }
    void setExchangeRate(double r) { m_exchangeRate = r; }
};
