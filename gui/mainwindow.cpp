#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    m_asset_manager = new TransactionManager(this);
    connect(ui->createButton, &QPushButton::clicked,
            this, &MainWindow::onCreateButtonClicked);
    connect(ui->closeTransactionButton, &QPushButton::clicked,
            this, &MainWindow::onCloseTransactionButtonClicked);
    connect(ui->deletePositionButton, &QPushButton::clicked,
            this, &MainWindow::onDeletePositionButtonClicked);
    connect(ui->calculatePotentialTaxButton, &QPushButton::clicked,
            this, &MainWindow::onPotentialCalculateButtonClicked);
    connect(m_asset_manager, &TransactionManager::databaseReady,
            this, &MainWindow::onDatabaseReady);
    connect(ui->cleanSelectionButton, &QPushButton::clicked,
            this, &MainWindow::onCleanSelectionButtonClicked);
    connect(m_asset_manager, &TransactionManager::fetchFailed,
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
        Transaction selectedTransaction = m_asset_manager->findTransactionById(item->text().toInt());
        ui->symbolLabel->setText(QString::fromStdString(selectedTransaction.getSymbol()));
        ui->quantityLabel->setText(QString::number(selectedTransaction.getQuantity()));
        ui->IDlabel->setText(QString::number(selectedTransaction.getId()));
    });
    
    qobject_cast<QHBoxLayout*>(ui->horizontalLayout_4->layout())->insertWidget(0, &m_table);
    m_table.refresh(m_asset_manager->getTransactions());

    calculateTotalTaxBase();
}

MainWindow::~MainWindow() {
    delete m_asset_manager;
    delete ui;
}

void MainWindow::onDatabaseReady() {
    qDebug() << "Database is ready, refreshing table.";
    m_table.refresh(m_asset_manager->getTransactions());
    calculateTotalTaxBase();
}

void MainWindow::onCleanSelectionButtonClicked() {
    ui->symbolLabel->clear();
    ui->quantityLabel->clear();
    ui->IDlabel->clear();
}

void MainWindow::onCreateButtonClicked() {
    if (!m_create_dialog) {
        m_create_dialog = std::make_unique<CreateDialog>(this);
        connect(m_create_dialog.get(), &CreateDialog::assetCreated,
                m_asset_manager, &TransactionManager::openTransaction, Qt::SingleShotConnection);

        m_create_dialog->exec();
        m_create_dialog.reset();
    }
}

void MainWindow::onDeletePositionButtonClicked() {
    if(ui->IDlabel->text().isEmpty()) {
        QMessageBox::warning(this, "Pozisyon Sil", "Silinecek pozisyon seçilmedi.");
        return;
    }

    int id = ui->IDlabel->text().toInt();
                
    try{
        m_asset_manager->removeTransaction(id);
    } catch (const std::runtime_error& e) {
        QMessageBox::warning(this, "Pozisyon Sil", e.what());
        return;
    }
    QMessageBox::information(this, "Pozisyon Sil", "Pozisyon başarıyla silindi.");
}

void MainWindow::onCloseTransactionButtonClicked() {
    if (ui->IDlabel->text().isEmpty()) {
        return; // No selection
    }

    Transaction selectedTransaction = m_asset_manager->findTransactionById(ui->IDlabel->text().toInt());
    double sellPrice = ui->sellPriceSpinBox->value();
    QDate sellDate = ui->sellDateEdit->date();

    if (sellPrice <= 0 || !sellDate.isValid()) {
        QMessageBox::warning(this, "Giriş Hatası", "Geçerli bir satış fiyatı veya tarih giriniz.");
        return;
    }

    selectedTransaction.setSellDate(sellDate);
    selectedTransaction.setSellPrice(sellPrice);
    selectedTransaction.setStatus("Kapalı");

    m_asset_manager->closeTransaction(selectedTransaction);
}

void MainWindow::onPotentialCalculateButtonClicked() {
    if (ui->IDlabel->text().isEmpty()) {
        QMessageBox::warning(this, "Seçim Hatası", "Potansiyel vergi hesaplamak için lütfen bir pozisyon seçin.");
        return;
    }

    Transaction selectedTransaction = m_asset_manager->findTransactionById(ui->IDlabel->text().toInt());
    double potentialSellPrice = ui->potentialSellPriceSpinBox->value();
    QDate currentDate = QDate::currentDate();

    if (potentialSellPrice <= 0) {
        QMessageBox::warning(this, "Giriş Hatası", "Lütfen geçerli bir satış fiyatı girin.");
        return;
    }

    selectedTransaction.setSellDate(currentDate);
    selectedTransaction.setSellPrice(potentialSellPrice);

    connect(m_asset_manager, &TransactionManager::potentialTaxBaseReady,
            this, [this](double potentialTaxBase) {
                ui->potentialCalculatedTaxLabel->setText(QString::number(potentialTaxBase, 'f', 2) + " ₺");
                calculateTotalTaxBase(potentialTaxBase);
            }, Qt::SingleShotConnection);
    m_asset_manager->potentialTransaction(selectedTransaction);
}

void MainWindow::onFetchFailed(const QString &error) {
    QMessageBox::warning(this, "İşlem Başarısız", error);
}

void MainWindow::calculateTotalTaxBase(double potential) {
    double totalTaxBase = 0.0;
    
    for(const auto &transaction : m_asset_manager->getTransactions()) {
        if (transaction.getStatus() == "Kapalı") {
            QDate sellDate = transaction.getSellQDate();
            
            if(sellDate.year() == QDate::currentDate().year()) {
                totalTaxBase += transaction.getTaxBase();
            }
        }
    }

    totalTaxBase += potential;
    ui->totalTaxBaseLabel->setText(QString::number(totalTaxBase, 'f', 2) + " ₺");
}