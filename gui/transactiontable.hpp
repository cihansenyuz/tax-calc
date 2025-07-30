#pragma once

#include <QTableWidget>
#include "../inc/asset.hpp"

class TransactionTable : public QTableWidget {
public:
    explicit TransactionTable(QWidget *parent = nullptr)
        : QTableWidget(parent) {
        
        setColumnCount(m_columnCount);
        setHorizontalHeaderLabels(m_labels);
        resizeColumnsToContents();
    }

    void refresh(const std::vector<Asset> &assets) {
        clearContents();
        setRowCount(0);

        unsigned int rowCount = assets.size();
        setRowCount(rowCount);

        for (unsigned int currentRow = 0; currentRow < rowCount; currentRow++) {
            const Asset &asset = assets.at(currentRow);

        QTableWidgetItem *item = new QTableWidgetItem(QString::number(asset.getId()));
        item->setTextAlignment(Qt::AlignCenter);
        setItem(currentRow, 0, item);

        item = new QTableWidgetItem(QString::fromStdString(asset.getSymbol()));
        setItem(currentRow, 1, item);

        item = new QTableWidgetItem(QString::fromStdString(asset.getSymbolName()));
        setItem(currentRow, 2, item);

        item = new QTableWidgetItem(QString::number(asset.getQuantity()));
        item->setTextAlignment(Qt::AlignCenter);
        setItem(currentRow, 3, item);

        item = new QTableWidgetItem(asset.getBuyDate());
        item->setTextAlignment(Qt::AlignCenter);
        setItem(currentRow, 4, item);

        item = new QTableWidgetItem(QString::number(asset.getBuyPrice()));
        item->setTextAlignment(Qt::AlignCenter);
        setItem(currentRow, 5, item);

        item = new QTableWidgetItem(asset.getSellDate());
        item->setTextAlignment(Qt::AlignCenter);
        setItem(currentRow, 6, item);

        item = new QTableWidgetItem(QString::number(asset.getSellPrice()));
        item->setTextAlignment(Qt::AlignCenter);
        setItem(currentRow, 7, item);

        item = new QTableWidgetItem(asset.getStatus());
        item->setTextAlignment(Qt::AlignCenter);
        setItem(currentRow, 8, item);
        
        item = new QTableWidgetItem(QString::number(asset.getTaxBase()));
        item->setTextAlignment(Qt::AlignCenter);
        setItem(currentRow, 9, item);
        }

        resizeColumnsToContents();
    }

private:
    QStringList m_labels{
        "Kayıt No", "Sembol", "İsim", "Adet", "Alış Tarihi",
        "Alış Fiyatı", "Satış Tarihi", "Satış Fiyatı",
        "Durum", "Vergi Matrahı"
    };
    int m_columnCount = 10;
};