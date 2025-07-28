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

    connect(ui->createButton, &QPushButton::clicked, this, &MainWindow::onCreateButtonClicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onCreateButtonClicked()
{
    if (!m_create_dialog) {
        m_create_dialog = std::make_unique<CreateDialog>(this);
        connect(m_create_dialog.get(), &CreateDialog::assetCreated, this,
                [this](const Asset &new_asset) {
            m_asset_manager.addAsset(new_asset);
            qDebug() << "Asset created:" << new_asset.getSymbol()
                     << new_asset.getSymbolName() << new_asset.getBuyPrice();

            qDebug() << "Total assets managed:" << m_asset_manager.size();
        });

        m_create_dialog->exec();
        m_create_dialog.reset();
    }
}