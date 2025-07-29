#include "mainwindow.hpp"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_asset_manager = new AssetManager(this);
    connect(ui->createButton, &QPushButton::clicked, this, &MainWindow::onCreateButtonClicked);

    ui->tableWidget->setColumnCount(8);
    QStringList labels;
    labels << "Sembol"
        << "İsim"
        << "Adet"
        << "Alış Tarihi"
        << "Alış Fiyatı"
        << "Satış Tarihi"
        << "Satış Fiyatı"
        << "Durum";
    ui->tableWidget->setHorizontalHeaderLabels(labels);
    ui->tableWidget->resizeColumnsToContents();
}

MainWindow::~MainWindow()
{
    delete m_asset_manager;
    delete ui;
}

void MainWindow::onCreateButtonClicked()
{
    if (!m_create_dialog) {
        m_create_dialog = std::make_unique<CreateDialog>(this);
        connect(m_create_dialog.get(), &CreateDialog::assetCreated, this,
                [this](const Asset &new_asset) {
            m_asset_manager->addAsset(new_asset);
        });
        m_create_dialog->exec();
        m_create_dialog.reset();

        connect(m_asset_manager, &AssetManager::assetUpdated,
                this, &MainWindow::updateTable, Qt::SingleShotConnection);
    }
}

void MainWindow::updateTable()
{
    ui->tableWidget->clearContents();

    unsigned int rowCount = m_asset_manager->size();
    ui->tableWidget->setRowCount(rowCount);

    for (unsigned int currentRow = 0; currentRow < rowCount; currentRow++)
    {
        const Asset &asset = m_asset_manager->getAssets().at(currentRow);

        QTableWidgetItem *item = new QTableWidgetItem(QString::fromStdString(asset.getSymbol()));
        ui->tableWidget->setItem(currentRow, 0, item);

        item = new QTableWidgetItem(QString::fromStdString(asset.getSymbolName()));
        ui->tableWidget->setItem(currentRow, 1, item);

        item = new QTableWidgetItem(QString::number(asset.getQuantity()));
        item->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(currentRow, 2, item);

        item = new QTableWidgetItem(asset.getBuyDate());
        item->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(currentRow, 3, item);

        item = new QTableWidgetItem(QString::number(asset.getBuyPrice()));
        item->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(currentRow, 4, item);

        item = new QTableWidgetItem(asset.getSellDate());
        item->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(currentRow, 5, item);

        item = new QTableWidgetItem(QString::number(asset.getSellPrice()));
        item->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(currentRow, 6, item);

        item = new QTableWidgetItem(asset.getStatus());
        item->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(currentRow, 7, item);
    }

    ui->tableWidget->resizeColumnsToContents();
}