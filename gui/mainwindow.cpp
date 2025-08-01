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
    connect(m_asset_manager, &AssetManager::fetchFailed,
            this, &MainWindow::onFetchFailed);

    connect(ui->selectButton, &QPushButton::clicked, this, [this]() {
        int selectedRow = m_table.currentRow();
        if (selectedRow < 0) {
            QMessageBox::warning(this, "Seçim Hatası", "Lütfen kapatılacak bir işlem seçin.");
            return;
        }
        QTableWidgetItem *item = m_table.item(selectedRow, 0);
        if (!item) {
            QMessageBox::warning(this, "Seçim Hatası", "Geçerli bir işlem seçilmedi.");
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
        QMessageBox::warning(this, "Pozisyon Sil", "Silinecek pozisyon seçilmedi.");
        return;
    }

    int id = ui->IDlabel->text().toInt();
                
    try{
        m_asset_manager->removeAsset(id);
    } catch (const std::runtime_error& e) {
        QMessageBox::warning(this, "Pozisyon Sil", e.what());
        return;
    }
    QMessageBox::information(this, "Pozisyon Sil", "Pozisyon başarıyla silindi.");
}

void MainWindow::onCloseTransactionButtonClicked(){
    if (ui->IDlabel->text().isEmpty()) {
        return; // No selection
    }

    Asset selectedAsset = m_asset_manager->findAssetById(ui->IDlabel->text().toInt());
    double sellPrice = ui->sellPriceSpinBox->value();
    QDate sellDate = ui->sellDateEdit->date();

    if (sellPrice <= 0 || !sellDate.isValid()) {
        QMessageBox::warning(this, "Giriş Hatası", "Geçerli bir satış fiyatı veya tarih giriniz.");
        return;
    }

    selectedAsset.setSellDate(sellDate);
    selectedAsset.setSellPrice(sellPrice);
    selectedAsset.setStatus("Kapalı");

    m_asset_manager->closeTransaction(selectedAsset);
}

void MainWindow::onPotentialCalculateButtonClicked(){
    if (ui->IDlabel->text().isEmpty()) {
        QMessageBox::warning(this, "Seçim Hatası", "Potansiyel vergi hesaplamak için lütfen bir pozisyon seçin.");
        return;
    }

    Asset selectedAsset = m_asset_manager->findAssetById(ui->IDlabel->text().toInt());
    double potentialSellPrice = ui->potentialSellPriceSpinBox->value();
    QDate currentDate = QDate::currentDate();

    if (potentialSellPrice <= 0) {
        QMessageBox::warning(this, "Giriş Hatası", "Lütfen geçerli bir satış fiyatı girin.");
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

void MainWindow::onFetchFailed(const QString &error) {
    QMessageBox::warning(this, "İşlem Başarısız", error);
}