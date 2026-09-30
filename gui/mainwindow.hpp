#pragma once

#include <QMainWindow>
#include <QProgressDialog>
#include <vector>
#include <queue>
#include <functional>

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
    void onResetPotCalcButtonClicked();
    void abortClose(const QString &error);
    void abortPotentialCalc(const QString &error);

private:
    Ui::MainWindow *ui;
    std::unique_ptr<CreateDialog> m_create_dialog;
    TransactionManager *transaction_manager;
    TransactionTable m_table{this};
    std::vector<Transaction> m_selectedTransactions; // the transactions currently selected in the table

    struct DrainContext { // A context struct for managing multiple sequential operations
        std::queue<Transaction> queue;
        int total = 0;
        QProgressDialog *progressDialog = nullptr;
        int pendingId = 0;
        QMetaObject::Connection nextConnection;
        std::function<void(const Transaction &)> action;
        std::function<void()> onDone;
        std::function<QMetaObject::Connection(std::function<void()>)> connectAdvance;
    };
    DrainContext m_closeDrain;
    DrainContext m_deleteDrain;
    DrainContext m_potentialDrain;
    double m_potentialAccumulator = 0.0;

    void runNext(DrainContext &ctx);
    void calculateTotalTaxBase(double potential = 0.0);
};
