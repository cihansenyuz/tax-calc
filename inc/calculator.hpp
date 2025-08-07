#pragma once

#include "transaction.hpp"

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
            qDebug() << "Error calculating tax base: " << e.what();
            return 0.0;
        }
    }
};