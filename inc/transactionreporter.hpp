#pragma once

#include <QString>
#include "transaction.hpp"
#include "transactionmanager.hpp"
#include "calculator.hpp"

class TransactionReporter {
public:
    static void writeReport(const Transaction& transaction,
                             TransactionManager::TransactionType action,
                             const FetchResult& buyData,
                             const FetchResult& sellData,
                             const TaxCalculationBreakdown& breakdown);
private:
    static QString actionToString(TransactionManager::TransactionType action);
    static QString buildReportText(const Transaction& transaction,
                                    TransactionManager::TransactionType action,
                                    const FetchResult& buyData,
                                    const FetchResult& sellData,
                                    const TaxCalculationBreakdown& breakdown);
};
