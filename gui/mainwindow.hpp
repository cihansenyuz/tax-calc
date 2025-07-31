#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>

#include "../inc/assetmanager.hpp"
#include "../inc/network/evdsfetcher.h"
#include "../inc/network/httpmanager.h"
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
    void onDatabaseReady();

private:
    Ui::MainWindow *ui;
    std::unique_ptr<CreateDialog> m_create_dialog;
    AssetManager *m_asset_manager;
    TransactionTable m_table{this};
};

#endif // MAINWINDOW_HPP
