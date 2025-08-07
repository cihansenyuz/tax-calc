#pragma once

#include <memory>
#include <QDialog>
#include <QTimer>
#include <QCompleter>
#include "../inc/transaction.hpp"
#include "../inc/network/yahoofinancefetcher.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class Dialog; }
QT_END_NAMESPACE

class CreateDialog : public QDialog {
    Q_OBJECT

public:
    explicit CreateDialog(QWidget *parent = nullptr);
    ~CreateDialog();

    QString symbol() const;
    QString name() const;
    double buyPrice() const;
    QDate buyDate() const;

signals:
    void assetCreated(const Transaction &new_transaction);

private slots:
    void onOkClicked();
    void onAssetSelected(const QString &selection);

private:
    Ui::Dialog *ui;
    std::unique_ptr<QTimer> m_searchTimer;
    std::unique_ptr<YahooFinanceFetcher> m_fetcher;
    std::unique_ptr<QCompleter> m_symbolAndNameCompleter;
};
