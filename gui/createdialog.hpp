#pragma once

#include <QDialog>
#include "../inc/transaction.hpp"

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

private:
    Ui::Dialog *ui;
};
