#pragma once

#include "transaction.hpp"
#include "logger.hpp"

struct TaxCalculationBreakdown {
    double sellPriceTry = 0.0;
    double buyPriceTry = 0.0;
    double inflationScaler = 0.0;
    bool inflationAdjustmentApplied = false; // scaler >= 1.1 threshold
    double rawTaxBase = 0.0;                  // before clamping to 0
    double taxBase = 0.0;                     // after clamping to 0
    QString error;                            // non-empty if calculation failed
};

class Calculator {
public:
    static double calculateTax(double taxBase, double taxRate, double declaretionLimit) {
        double tax = 0.0;
        if (taxBase > declaretionLimit) {
            tax = taxBase * taxRate;
        }
        return tax;
    }

    static struct TaxCalculationBreakdown calculateTaxBaseDetailed(const Transaction &transaction) {
        TaxCalculationBreakdown b;
        b.sellPriceTry = transaction.getExchangeRateAtSell() * (transaction.getSellPrice() * transaction.getQuantity() - 1.5);
        b.buyPriceTry  = transaction.getExchangeRateAtBuy()  * (transaction.getBuyPrice()  * transaction.getQuantity() + 1.5);
        try {
            b.inflationScaler = transaction.getInflationIndexAtSell() / transaction.getInflationIndexAtBuy();
            b.inflationAdjustmentApplied = b.inflationScaler >= 1.1;
            b.rawTaxBase = b.inflationAdjustmentApplied
                ? (b.sellPriceTry - (b.buyPriceTry * b.inflationScaler))
                : (b.sellPriceTry - b.buyPriceTry);
            b.taxBase = b.rawTaxBase < 0 ? 0.0 : b.rawTaxBase;
        } catch (const std::exception& e) {
            b.error = e.what();
            qWarning(logCalculator) << "Error calculating tax base: " << e.what();
        }
        return b;
    }

    static double calculateTaxBase(const Transaction &transaction) {
        return calculateTaxBaseDetailed(transaction).taxBase; // keeps existing callers unchanged
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