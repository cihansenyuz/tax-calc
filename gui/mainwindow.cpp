#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "../inc/calculator.hpp"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    QString appName = QCoreApplication::applicationName();
    QString version = QCoreApplication::applicationVersion();
    setWindowTitle(QString("%1 v%2").arg(appName, version));

    transaction_manager = new TransactionManager(this);
    connect(ui->createButton, &QPushButton::clicked,
            this, &MainWindow::onCreateButtonClicked);
    connect(ui->closeTransactionButton, &QPushButton::clicked,
            this, &MainWindow::onCloseTransactionButtonClicked);
    connect(ui->deletePositionButton, &QPushButton::clicked,
            this, &MainWindow::onDeletePositionButtonClicked);
    connect(ui->calculatePotentialTaxButton, &QPushButton::clicked,
            this, &MainWindow::onPotentialCalculateButtonClicked);
    connect(transaction_manager, &TransactionManager::databaseReady,
            this, &MainWindow::onDatabaseReady);
    connect(ui->cleanSelectionButton, &QPushButton::clicked,
            this, &MainWindow::onCleanSelectionButtonClicked);
    connect(transaction_manager, &TransactionManager::fetchFailed,
            this, &MainWindow::onFetchFailed);
    connect(ui->selectButton, &QPushButton::clicked,
            this, &MainWindow::onSelectButtonClicked);
    
    qobject_cast<QHBoxLayout*>(ui->horizontalLayout_4->layout())->insertWidget(0, &m_table);
    m_table.refresh(transaction_manager->getTransactions());
    
    ui->sellDateEdit->setDate(QDate::currentDate());
    ui->taxRangesComboBox->addItem("15%", 0.15);
    ui->taxRangesComboBox->addItem("20%", 0.20);
    ui->taxRangesComboBox->addItem("27%", 0.27);
    ui->taxRangesComboBox->addItem("35%", 0.35);
    ui->taxRangesComboBox->addItem("40%", 0.40);
    connect(ui->taxRangesComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this]() {
               if(ui->potentialCalculatedTaxLabel->text().isEmpty())
                    calculateTotalTaxBase();
                else
                    calculateTotalTaxBase(ui->potentialCalculatedTaxLabel->text().toDouble());
            });

    calculateTotalTaxBase();
}

MainWindow::~MainWindow() {
    delete transaction_manager;
    delete ui;
}

void MainWindow::onDatabaseReady() {
    // qDebug() << "Database is ready, refreshing table.";
    m_table.refresh(transaction_manager->getTransactions());
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
                transaction_manager, &TransactionManager::openTransaction, Qt::SingleShotConnection);

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
        transaction_manager->removeTransaction(id);
    } catch (const std::runtime_error& e) {
        QMessageBox::warning(this, "Pozisyon Sil", e.what());
        return;
    }
    QMessageBox::information(this, "Pozisyon Sil", "Pozisyon başarıyla silindi.");
}

void MainWindow::onCloseTransactionButtonClicked() {
    if (ui->IDlabel->text().isEmpty()) {
        QMessageBox::warning(this, "Giriş Hatası", "Kapatılacak pozisyon seçilmedi.");
        return;
    }

    Transaction selectedTransaction = transaction_manager->findTransactionById(ui->IDlabel->text().toInt());
    double sellPrice = ui->sellPriceSpinBox->value();
    QDate sellDate = ui->sellDateEdit->date();

    if (sellPrice <= 0 || !sellDate.isValid()) {
        QMessageBox::warning(this, "Giriş Hatası", "Geçerli bir satış fiyatı veya tarih giriniz.");
        return;
    }

    selectedTransaction.setSellDate(sellDate);
    selectedTransaction.setSellPrice(sellPrice);
    selectedTransaction.setStatus(Transaction::Status::Closed);

    transaction_manager->closeTransaction(selectedTransaction);
}

void MainWindow::onPotentialCalculateButtonClicked() {
    if (ui->IDlabel->text().isEmpty()) {
        QMessageBox::warning(this, "Seçim Hatası", "Potansiyel vergi hesaplamak için lütfen bir pozisyon seçin.");
        return;
    }

    Transaction selectedTransaction = transaction_manager->findTransactionById(ui->IDlabel->text().toInt());
    double potentialSellPrice = ui->potentialSellPriceSpinBox->value();
    QDate currentDate = QDate::currentDate();

    if (potentialSellPrice <= 0) {
        QMessageBox::warning(this, "Giriş Hatası", "Lütfen geçerli bir satış fiyatı girin.");
        return;
    }

    selectedTransaction.setSellDate(currentDate);
    selectedTransaction.setSellPrice(potentialSellPrice);

    connect(transaction_manager, &TransactionManager::potentialTaxBaseReady,
            this, [this](double potentialTaxBase) {
                ui->potentialCalculatedTaxLabel->setText(QString::number(potentialTaxBase, 'f', 2) + " ₺");
                calculateTotalTaxBase(potentialTaxBase);
            }, Qt::SingleShotConnection);
    transaction_manager->potentialTransaction(selectedTransaction);
}

void MainWindow::onFetchFailed(const QString &error) {
    QMessageBox::warning(this, "İşlem Başarısız", error);
}

void MainWindow::calculateTotalTaxBase(double potential) {
    double totalTaxBase = 0.0;
    if(potential != 0.0)
        ui->totalTaxBaseLabelText->setText("Toplam Kazancım\n(Potansiyel dahil)");
    else
        ui->totalTaxBaseLabelText->setText("Toplam Kazancım");
    
    for(const auto &transaction : transaction_manager->getTransactions()) {
        if (transaction.getStatus() == Transaction::Status::Closed) {
            QDate sellDate = transaction.getSellQDate();
            
            if(sellDate.year() == QDate::currentDate().year()) {
                totalTaxBase += transaction.getTaxBase();
            }
        }
    }

    totalTaxBase += potential;
    ui->totalTaxBaseLabel->setText(QString::number(totalTaxBase, 'f', 2) + " ₺");

    double calculatedTax = Calculator::calculateTax(totalTaxBase,
                            ui->taxRangesComboBox->currentData().toDouble(),
                            ui->declaretionLimitSpinBox->value());
    ui->calculatedTaxLabel->setText(QString::number(calculatedTax, 'f', 2) + " ₺");
}

void MainWindow::onSelectButtonClicked() {
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
    Transaction selectedTransaction = transaction_manager->findTransactionById(item->text().toInt());
    ui->symbolLabel->setText(QString::fromStdString(selectedTransaction.getSymbol()));
    ui->quantityLabel->setText(QString::number(selectedTransaction.getQuantity()));
    ui->IDlabel->setText(QString::number(selectedTransaction.getId()));
}
