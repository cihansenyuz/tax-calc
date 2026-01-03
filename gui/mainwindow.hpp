#pragma once

#include <QMainWindow>

#include "../inc/transactionmanager.hpp"
#include "../inc/network/evdsfetcher.hpp"
#include "../inc/network/httpmanager.hpp"
#include "transactiontable.hpp"
#include "createdialog.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onCreateButtonClicked();
    void onCloseTransactionButtonClicked();
    void onDeletePositionButtonClicked();
    void onPotentialCalculateButtonClicked();
    void onDatabaseReady();
    void onCleanSelectionButtonClicked();
    void onFetchFailed(const QString &error);
    void onSelectButtonClicked();

private:
    Ui::MainWindow *ui;
    std::unique_ptr<CreateDialog> m_create_dialog;
    TransactionManager *transaction_manager;
    TransactionTable m_table{this};

    void calculateTotalTaxBase(double potential = 0.0);
};
