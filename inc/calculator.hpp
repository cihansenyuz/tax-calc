#pragma once

#include "asset.hpp"

class Calculator {
public:
    static double calculateTax(const Asset &asset){
        double tax = 0.0;
        // taxBase = taxbase * taxRate
        // TO DO: learn taxRate ranges and implement calculation
        return tax;
    }

    static double calculateTaxBase(const Asset &asset){
        double sellPrice = asset.getExchangeRateAtSell() * (asset.getSellPrice() * asset.getQuantity() - 1.5);
        double buyPrice = asset.getExchangeRateAtBuy() * (asset.getBuyPrice() * asset.getQuantity() + 1.5);

        try{
            double inflationScaler = asset.getInflationIndexAtSell() / asset.getInflationIndexAtBuy();
            double taxBase = 0.0;

            if(inflationScaler >= 1.1)
                taxBase = (sellPrice - (buyPrice * inflationScaler));
            else
                taxBase = (sellPrice - buyPrice);

            return taxBase;
        }
        catch (const std::exception& e) {
            qDebug() << "Error calculating tax base: " << e.what();
            return 0.0;
        }
    }
};