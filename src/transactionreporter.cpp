#include "../inc/transactionreporter.hpp"
#include "../inc/logger.hpp"
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

QString TransactionReporter::actionToString(TransactionManager::TransactionType action) {
    switch (action) {
        case TransactionManager::TransactionType::Open:      return "Open Position";
        case TransactionManager::TransactionType::Close:     return "Close Position";
        case TransactionManager::TransactionType::Potential: return "Potential Tax Base Calculation";
        default:                                              return "Unknown";
    }
}

QString TransactionReporter::buildReportText(const Transaction& transaction,
                                              TransactionManager::TransactionType action,
                                              const FetchResult& buyData,
                                              const FetchResult& sellData,
                                              const TaxCalculationBreakdown& breakdown) {
    QString text;
    QTextStream out(&text);

    out << "=== Tax Calculation Report ===\n";
    if (action == TransactionManager::TransactionType::Potential)
        out << "*** POTENTIAL CALCULATION - NOT SAVED ***\n";
    out << "Transaction ID: " << transaction.getId() << " (" << QString::fromStdString(transaction.getSymbol()) << ")\n";
    out << "Action: " << actionToString(action) << "\n";
    out << "Generated: " << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << "\n\n";

    out << "Buy date: " << transaction.getBuyDate() << "\n";
    out << "  Exchange rate (USD/TRY): " << Calculator::formatCurrency(buyData.m_exchangeRate.value)
        << " (recorded when position was opened)\n";
    out << "  Inflation index: " << Calculator::formatCurrency(buyData.m_inflationIndex.value)
        << " (recorded when position was opened)\n";

    // Sell side and tax base breakdown only apply once a sell date/price exist
    if (action != TransactionManager::TransactionType::Open) {
        out << "\nSell date: " << transaction.getSellDate() << "\n";
        out << "  Exchange rate (USD/TRY): " << Calculator::formatCurrency(sellData.m_exchangeRate.value)
            << " (data date: "
            << (sellData.m_exchangeRate.date.isValid() ? sellData.m_exchangeRate.date.toString("dd-MM-yyyy") : QString("unknown"))
            << ")\n";
        out << "  Inflation index: " << Calculator::formatCurrency(sellData.m_inflationIndex.value)
            << " (data date: "
            << (sellData.m_inflationIndex.date.isValid() ? sellData.m_inflationIndex.date.toString("dd-MM-yyyy") : QString("unknown"))
            << ")\n\n";

        out << "Sell price (TRY): " << Calculator::formatCurrency(breakdown.sellPriceTry) << "\n";
        out << "Buy price (TRY): " << Calculator::formatCurrency(breakdown.buyPriceTry) << "\n";
        out << "Inflation scaler: " << QString::number(breakdown.inflationScaler, 'f', 4)
            << (breakdown.inflationAdjustmentApplied
                    ? " (>= 1.10 threshold -> inflation adjustment applied)\n"
                    : " (< 1.10 threshold -> inflation adjustment NOT applied)\n");
        out << "Tax base: " << Calculator::formatCurrency(breakdown.taxBase) << "\n";
        if (!breakdown.error.isEmpty())
            out << "Calculation error: " << breakdown.error << "\n";
    }

    return text;
}

void TransactionReporter::writeReport(const Transaction& transaction,
                                       TransactionManager::TransactionType action,
                                       const FetchResult& buyData,
                                       const FetchResult& sellData,
                                       const TaxCalculationBreakdown& breakdown) {
    QDir().mkpath("reports");

    QString actionTag = action == TransactionManager::TransactionType::Open      ? "open"
                      : action == TransactionManager::TransactionType::Close     ? "close"
                                                                                  : "potential";
    QString filename = QString("reports/%1_%2_%3.txt")
        .arg(transaction.getId())
        .arg(actionTag)
        .arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"));

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning(logManager) << "Failed to write transaction report:" << filename;
        return;
    }
    QTextStream stream(&file);
    stream << buildReportText(transaction, action, buyData, sellData, breakdown);
}
