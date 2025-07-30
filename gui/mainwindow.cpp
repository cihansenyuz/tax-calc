#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_asset_manager = new AssetManager(this);
    connect(ui->createButton, &QPushButton::clicked, this, &MainWindow::onCreateButtonClicked);
    connect(ui->closeTransactionButton, &QPushButton::clicked, this, &MainWindow::onCloseTransactionButtonClicked);
    connect(ui->selectButton, &QPushButton::clicked, this, [this]() {
        int selectedRow = m_table.currentRow();
        if (selectedRow < 0) {
            QMessageBox::warning(this, "Selection Error", "Please select a transaction to close.");
            return;
        }
        QTableWidgetItem *item = m_table.item(selectedRow, 8);
        if (!item) {
            QMessageBox::warning(this, "Selection Error", "No valid transaction selected.");
            return;
        }
        Asset selectedAsset = m_asset_manager->findAssetById(item->text().toInt());
        ui->symbolLabel->setText(QString::fromStdString(selectedAsset.getSymbol()));
        ui->quantityLabel->setText(QString::number(selectedAsset.getQuantity()));
        ui->IDlabel->setText(QString::number(selectedAsset.getId()));
    });
    
    qobject_cast<QVBoxLayout*>(ui->centralwidget->layout())->insertWidget(0, &m_table);
    m_table.refresh(m_asset_manager->getAssets());
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
        connect(m_create_dialog.get(), &CreateDialog::assetCreated,
                m_asset_manager, &AssetManager::openTransaction, Qt::SingleShotConnection);
        connect(m_asset_manager, &AssetManager::databaseReady,
                this, [this](){ 
                    qDebug() << "Database is ready, refreshing table.";
                    m_table.refresh(m_asset_manager->getAssets()); }, Qt::SingleShotConnection);

        m_create_dialog->exec();
        m_create_dialog.reset();
    }
}

void MainWindow::onCloseTransactionButtonClicked(){
    if (ui->IDlabel->text().isEmpty()) {
        return; // No selection
    }

    Asset selectedAsset = m_asset_manager->findAssetById(ui->IDlabel->text().toInt());
    double sellPrice = ui->sellPriceSpinBox->value();
    QDate sellDate = ui->sellDateEdit->date();

    if (sellPrice <= 0 || !sellDate.isValid()) {
        throw std::invalid_argument("Invalid sell price or date");
    }

    selectedAsset.setSellDate(sellDate);
    selectedAsset.setSellPrice(sellPrice);
    selectedAsset.setStatus("Kapalı");

    connect(m_asset_manager, &AssetManager::databaseReady,
            this, [this]() { 
                qDebug() << "Database is ready, refreshing table.";
                m_table.refresh(m_asset_manager->getAssets()); }, Qt::SingleShotConnection);

    m_asset_manager->closeTransaction(selectedAsset);
}