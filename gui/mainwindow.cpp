#include "mainwindow.hpp"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_http_manager = new HttpManager();
    m_http_manager->setKey(API_KEY);

    m_evds_fetcher = new EvdsFetcher(m_http_manager, this);
    QObject::connect(m_evds_fetcher, &EvdsFetcher::evdsDataFetched, this,
                        [](const std::shared_ptr<QJsonObject> &fetched_data) {
        if (fetched_data) {
            QFile file("fetched_data.json");
            if (file.open(QIODevice::WriteOnly)) {
                QJsonDocument doc(*fetched_data);
                file.write(doc.toJson());
                file.close();
            }
        }
    });

    m_evds_fetcher->fetch(EvdsFetcher::USD, "21-04-2024", "23-04-2024");
}

MainWindow::~MainWindow()
{
    delete ui;
}
