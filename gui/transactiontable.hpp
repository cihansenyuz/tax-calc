#pragma once

#include <QTableWidget>
#include <QDate>
#include "../inc/transaction.hpp"
#include "../inc/logger.hpp"
#include "../inc/calculator.hpp"

// Custom item class that sorts dates correctly
class DateTableWidgetItem : public QTableWidgetItem {
public:
    explicit DateTableWidgetItem(const QString& dateStr, const QDate& date)
        : QTableWidgetItem(dateStr), m_date(date) {}
    
    bool operator<(const QTableWidgetItem& other) const override {
        const DateTableWidgetItem* dateItem = dynamic_cast<const DateTableWidgetItem*>(&other);
        if (dateItem) {
            return m_date < dateItem->m_date;
        }
        return QTableWidgetItem::operator<(other);
    }

private:
    QDate m_date;
};

class TransactionTable : public QTableWidget {
public:
    explicit TransactionTable(QWidget *parent = nullptr)
        : QTableWidget(parent) {

    setColumnCount(m_columnCount);
    setHorizontalHeaderLabels(m_labels);
    setMinimumSize(800, 400);
    resizeColumnsToContents();
    setSortingEnabled(true);

    connect(this, &QTableWidget::itemSelectionChanged, this, [this]() {
        m_selectedIds.clear();
        QSet<int> seenRows;
        for (QTableWidgetItem *item : selectedItems()) {
            int row = item->row();
            if (seenRows.contains(row)) continue;
            seenRows.insert(row);
            QTableWidgetItem *idItem = this->item(row, 0);
            if (idItem) m_selectedIds.append(idItem->text());
        }
        qCDebug(logGui) << "Selected IDs:" << m_selectedIds;
    });
}

    const QList<QString>& selectedIds() const { return m_selectedIds; }

    void refresh(const std::vector<Transaction> &transactions) {
        setSortingEnabled(false);  // Disable during refresh
        
        clearContents();
        setRowCount(0);

        unsigned int rowCount = transactions.size();
        setRowCount(rowCount);

        for (unsigned int currentRow = 0; currentRow < rowCount; currentRow++) {
            const Transaction &transaction = transactions.at(currentRow);

        QTableWidgetItem *item = new QTableWidgetItem(QString::number(transaction.getId()));
        item->setTextAlignment(Qt::AlignCenter);
        setItem(currentRow, 0, item);

        item = new QTableWidgetItem(QString::fromStdString(transaction.getSymbol()));
        setItem(currentRow, 1, item);

        item = new QTableWidgetItem(QString::fromStdString(transaction.getSymbolName()));
        setItem(currentRow, 2, item);

        item = new QTableWidgetItem(QString::number(transaction.getQuantity(), 'g', 15));
        item->setTextAlignment(Qt::AlignCenter);
        setItem(currentRow, 3, item);

        item = new DateTableWidgetItem(transaction.getBuyDate(), transaction.getBuyQDate());
        item->setTextAlignment(Qt::AlignCenter);
        setItem(currentRow, 4, item);

        item = new QTableWidgetItem(QString::number(transaction.getBuyPrice()));
        item->setTextAlignment(Qt::AlignCenter);
        setItem(currentRow, 5, item);

        item = new DateTableWidgetItem(transaction.getSellDate(), transaction.getSellQDate());
        item->setTextAlignment(Qt::AlignCenter);
        setItem(currentRow, 6, item);

        item = new QTableWidgetItem(QString::number(transaction.getSellPrice()));
        item->setTextAlignment(Qt::AlignCenter);
        setItem(currentRow, 7, item);

        item = new QTableWidgetItem(Transaction::statusToString(transaction.getStatus()));
        item->setTextAlignment(Qt::AlignCenter);
        setItem(currentRow, 8, item);
        
        item = new QTableWidgetItem(Calculator::formatCurrency(transaction.getTaxBase()), transaction.getTaxBase());
        item->setTextAlignment(Qt::AlignCenter);
        setItem(currentRow, 9, item);
        }

        resizeColumnsToContents();
        setSortingEnabled(true);  // Re-enable after refresh
    }

private:
    QList<QString> m_selectedIds;
    QStringList m_labels{
        "Pozisyon No", "Sembol", "İsim", "Adet", "Alış Tarihi",
        "Alış Fiyatı", "Satış Tarihi", "Satış Fiyatı",
        "Durum", "Vergi Matrahı"
    };
    int m_columnCount = 10;
};