#pragma once

#include "asset.hpp"

class Calculator {
public:
    static double calculateTax(const Asset &asset){
        double tax = 0.0;
        // ((sell price - buy price) * (UFE YI at buy date / UFE YI at sell date)) / exchange at sell date
        // sell price = exchange rate at sell date * sell price
        // buy price = exchange rate at buy date * buy price + comission
        double sellPrice = asset.getExchangeRateAtSell() * asset.getSellPrice();
        double buyPrice = asset.getExchangeRateAtBuy() * asset.getBuyPrice(); // missing comission cost

        try{
            tax = (sellPrice - buyPrice) * (asset.getInflationIndexAtBuy() / asset.getInflationIndexAtSell());
            tax /= asset.getExchangeRateAtSell();
        }
        catch (const std::exception& e) {
            qDebug() << "Error calculating tax: " << e.what();
            return 0.0; // Return 0 if there's an error
        }

        return tax;
    }
};