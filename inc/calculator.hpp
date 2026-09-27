#pragma once

#include "transaction.hpp"
#include "logger.hpp"

class Calculator {
public:
    static double calculateTax(double taxBase, double taxRate, double declaretionLimit) {
        double tax = 0.0;
        if (taxBase > declaretionLimit) {
            tax = taxBase * taxRate;
        }
        return tax;
    }

    static double calculateTaxBase(const Transaction &transaction){
        double sellPrice = transaction.getExchangeRateAtSell() * (transaction.getSellPrice() * transaction.getQuantity() - 1.5);
        double buyPrice = transaction.getExchangeRateAtBuy() * (transaction.getBuyPrice() * transaction.getQuantity() + 1.5);

        try{
            double inflationScaler = transaction.getInflationIndexAtSell() / transaction.getInflationIndexAtBuy();
            double taxBase = 0.0;

            if(inflationScaler >= 1.1)
                taxBase = (sellPrice - (buyPrice * inflationScaler));
            else
                taxBase = (sellPrice - buyPrice);

            return taxBase < 0 ? 0.0 : taxBase;
        }
        catch (const std::exception& e) {
            qWarning(logCalculator) << "Error calculating tax base: " << e.what();
            return 0.0;
        }
    }

    static QString formatCurrency(double value) {
        QString str = QString::number(value, 'f', 2);
        int dotPos = str.indexOf('.');
        int insertPos = (dotPos == -1) ? str.length() : dotPos;
        int startPos = (str.startsWith('-') || str.startsWith('+')) ? 1 : 0;
        for (int i = insertPos - 3; i > startPos; i -= 3) {
            str.insert(i, ',');
        }
        return str;
    }
};