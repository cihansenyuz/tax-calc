#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_asset_manager = new AssetManager(this);
    connect(ui->createButton, &QPushButton::clicked,
            this, &MainWindow::onCreateButtonClicked);
    connect(ui->closeTransactionButton, &QPushButton::clicked,
            this, &MainWindow::onCloseTransactionButtonClicked);
    connect(ui->deletePositionButton, &QPushButton::clicked,
            this, &MainWindow::onDeletePositionButtonClicked);
    connect(ui->calculatePotentialTaxButton, &QPushButton::clicked,
            this, &MainWindow::onPotentialCalculateButtonClicked);
    connect(m_asset_manager, &AssetManager::databaseReady,
            this, &MainWindow::onDatabaseReady);
    connect(ui->cleanSelectionButton, &QPushButton::clicked,
            this, &MainWindow::onCleanSelectionButtonClicked);

    connect(ui->selectButton, &QPushButton::clicked, this, [this]() {
        int selectedRow = m_table.currentRow();
        if (selectedRow < 0) {
            QMessageBox::warning(this, "Selection Error", "Please select a transaction to close.");
            return;
        }
        QTableWidgetItem *item = m_table.item(selectedRow, 0);
        if (!item) {
            QMessageBox::warning(this, "Selection Error", "No valid transaction selected.");
            return;
        }
        Asset selectedAsset = m_asset_manager->findAssetById(item->text().toInt());
        ui->symbolLabel->setText(QString::fromStdString(selectedAsset.getSymbol()));
        ui->quantityLabel->setText(QString::number(selectedAsset.getQuantity()));
        ui->IDlabel->setText(QString::number(selectedAsset.getId()));
    });
    
    qobject_cast<QHBoxLayout*>(ui->horizontalLayout_4->layout())->insertWidget(0, &m_table);
    m_table.refresh(m_asset_manager->getAssets());
}

MainWindow::~MainWindow()
{
    delete m_asset_manager;
    delete ui;
}

void MainWindow::onDatabaseReady()
{
    qDebug() << "Database is ready, refreshing table.";
    m_table.refresh(m_asset_manager->getAssets());
}

void MainWindow::onCleanSelectionButtonClicked()
{
    ui->symbolLabel->clear();
    ui->quantityLabel->clear();
    ui->IDlabel->clear();
}

void MainWindow::onCreateButtonClicked()
{
    if (!m_create_dialog) {
        m_create_dialog = std::make_unique<CreateDialog>(this);
        connect(m_create_dialog.get(), &CreateDialog::assetCreated,
                m_asset_manager, &AssetManager::openTransaction, Qt::SingleShotConnection);

        m_create_dialog->exec();
        m_create_dialog.reset();
    }
}

void MainWindow::onDeletePositionButtonClicked()
{
    if(ui->IDlabel->text().isEmpty()) {
        QMessageBox::warning(this, "Delete Position", "No position selected to delete.");
        return;
    }

    int id = ui->IDlabel->text().toInt();
                
    try{
        m_asset_manager->removeAsset(id);
    } catch (const std::runtime_error& e) {
        QMessageBox::warning(this, "Delete Position", e.what());
        return;
    }
    QMessageBox::information(this, "Delete Position", "Position deleted successfully.");
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

    m_asset_manager->closeTransaction(selectedAsset);
}

void MainWindow::onPotentialCalculateButtonClicked(){
    if (ui->IDlabel->text().isEmpty()) {
        QMessageBox::warning(this, "Selection Error", "Please select a position to calculate potential tax.");
        return;
    }

    Asset selectedAsset = m_asset_manager->findAssetById(ui->IDlabel->text().toInt());
    double potentialSellPrice = ui->potentialSellPriceSpinBox->value();
    QDate currentDate = QDate::currentDate();

    if (potentialSellPrice <= 0) {
        QMessageBox::warning(this, "Input Error", "Please enter a valid sell price.");
        return;
    }

    selectedAsset.setSellDate(currentDate);
    selectedAsset.setSellPrice(potentialSellPrice);

    connect(m_asset_manager, &AssetManager::potentialTaxBaseReady,
            this, [this](double potentialTaxBase) {
                ui->potentialCalculatedTaxLabel->setText(QString::number(potentialTaxBase, 'f', 2));
            }, Qt::SingleShotConnection);
    m_asset_manager->potentialTransaction(selectedAsset);
}