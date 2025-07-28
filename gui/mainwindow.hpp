#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>

#include "../inc/asset_manager.hpp"
#include "../inc/network/evdsfetcher.h"
#include "../inc/network/httpmanager.h"
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

private:
    Ui::MainWindow *ui;

    std::unique_ptr<CreateDialog> m_create_dialog;

    AssetManager m_asset_manager;
    EvdsFetcher *m_evds_fetcher;
    static constexpr const char* API_KEY = "HSzat3MFdF";
    HttpManager *m_http_manager;
};

#endif // MAINWINDOW_HPP
