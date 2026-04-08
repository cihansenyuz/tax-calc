#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "../inc/calculator.hpp"
#include <QMessageBox>
#include <QTimer>

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
    connect(ui->resetPotCalcButton, &QPushButton::clicked,
            this, &MainWindow::onResetPotCalcButtonClicked);
    connect(ui->declaretionLimitSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
        this, [this]() {
        if(ui->potentialCalculatedTaxLabel->text().isEmpty())
            calculateTotalTaxBase();
        else
            calculateTotalTaxBase(ui->potentialCalculatedTaxLabel->text().toDouble());
        });
    
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
    if (m_closeProgressDialog) {
        m_closeProgressDialog->deleteLater();
        m_closeProgressDialog = nullptr;
    }
    delete transaction_manager;
    delete ui;
}

void MainWindow::onDatabaseReady() {
    qDebug() << "Database is ready, refreshing table.";
    m_table.refresh(transaction_manager->getTransactions());
    calculateTotalTaxBase();
}

void MainWindow::onCleanSelectionButtonClicked() {
    m_table.clearSelection();
    m_selectedTransactions.clear();
    ui->symbolLabel->clear();
    ui->quantityLabel->clear();
    ui->IDlabel->clear();
    onResetPotCalcButtonClicked();
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
    if(m_selectedTransactions.empty()) {
        QMessageBox::warning(this, "Pozisyon Sil", "Silinecek pozisyon seçilmedi.");
        return;
    }

    m_transactionsToDelete = {};
    for (const Transaction& t : m_selectedTransactions)
        m_transactionsToDelete.push(t);

    deleteNextTransaction();
}

void MainWindow::deleteNextTransaction() {
    if (m_transactionsToDelete.empty()) {
        QMessageBox::information(this, "Pozisyon Sil", "Seçili pozisyonlar başarıyla silindi.");
        onCleanSelectionButtonClicked();
        return;
    }

    Transaction t = m_transactionsToDelete.front();
    m_transactionsToDelete.pop();

    connect(transaction_manager, &TransactionManager::databaseReady,
            this, &MainWindow::deleteNextTransaction,
            static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::SingleShotConnection));

    qDebug() << "Deleting transaction with ID:" << t.getId();
    try {
        transaction_manager->removeTransaction(t.getId());
    } catch (const std::runtime_error& e) {
        disconnect(transaction_manager, &TransactionManager::databaseReady,
                   this, &MainWindow::deleteNextTransaction);
        QMessageBox::warning(this, "Pozisyon Sil", e.what());
        m_transactionsToDelete = {};
        onCleanSelectionButtonClicked();
    }
}

void MainWindow::onCloseTransactionButtonClicked() {
    if (m_selectedTransactions.empty()) {
        QMessageBox::warning(this, "Giriş Hatası", "Kapatılacak pozisyon seçilmedi.");
        return;
    }

    double sellPrice = ui->sellPriceSpinBox->value();
    QDate sellDate = ui->sellDateEdit->date();

    if (sellPrice <= 0 || !sellDate.isValid()) {
        QMessageBox::warning(this, "Giriş Hatası", "Geçerli bir satış fiyatı veya tarih giriniz.");
        return;
    }

    m_transactionsToClose = {};
    for (Transaction t : m_selectedTransactions) {
        t.setSellDate(sellDate);
        t.setSellPrice(sellPrice);
        t.setStatus(Transaction::Status::Closed);
        m_transactionsToClose.push(t);
    }

    m_closeTotal = static_cast<int>(m_transactionsToClose.size());
    m_closeProgressDialog = new QProgressDialog("Pozisyonlar kapatılıyor...", QString(), 0, m_closeTotal, this);
    m_closeProgressDialog->setWindowModality(Qt::WindowModal);
    m_closeProgressDialog->setMinimumDuration(0);
    m_closeProgressDialog->setValue(0);

    closeNextTransaction();
}

void MainWindow::closeNextTransaction() {
    if (m_transactionsToClose.empty()) {
        if (m_closeProgressDialog) {
            m_closeProgressDialog->setValue(m_closeTotal);
            m_closeProgressDialog->deleteLater();
            m_closeProgressDialog = nullptr;
        }
        onCleanSelectionButtonClicked();
        onResetPotCalcButtonClicked();
        return;
    }

    if (m_closeProgressDialog)
        m_closeProgressDialog->setValue(m_closeTotal - static_cast<int>(m_transactionsToClose.size()));

    Transaction t = m_transactionsToClose.front();
    m_transactionsToClose.pop();

    connect(transaction_manager, &TransactionManager::databaseReady,
            this, &MainWindow::closeNextTransaction, Qt::SingleShotConnection);

    connect(transaction_manager, &TransactionManager::fetchFailed,
            this, [this, t](const QString &error) {
                disconnect(transaction_manager, &TransactionManager::databaseReady,
                           this, &MainWindow::closeNextTransaction);
                if (m_closeProgressDialog) {
                    m_closeProgressDialog->deleteLater();
                    m_closeProgressDialog = nullptr;
                }
                QMessageBox::warning(this, "Pozisyon Kapat", error);
                m_transactionsToClose = {};
                onCleanSelectionButtonClicked();
                qCritical(logNetwork) << "Failed to close transaction due to EVDS API error with ID:"
                                      << t.getId() << "Error:" << error;
            }, Qt::SingleShotConnection);
                
    
    qDebug() << "Closing transaction with ID:" << t.getId();
    // Needs to be delayed to get respond from EVDS API
    QTimer::singleShot(1000, this, [this, t]() {
        transaction_manager->closeTransaction(t);
    });
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
    const QList<QString> &ids = m_table.selectedIds();
    if (ids.isEmpty()) {
        QMessageBox::warning(this, "Seçim Hatası", "Lütfen kapatılacak bir işlem seçin.");
        return;
    }

    m_selectedTransactions.clear();
    QStringList symbols, quantities, idStrings;
    for (const QString &idStr : ids) {
        Transaction selectedTransaction = transaction_manager->findTransactionById(idStr.toInt());
        m_selectedTransactions.push_back(selectedTransaction);
        symbols    << QString::fromStdString(selectedTransaction.getSymbol());
        quantities << QString::number(selectedTransaction.getQuantity());
        idStrings  << QString::number(selectedTransaction.getId());
    }

    ui->symbolLabel->setText(symbols.join("\n"));
    ui->quantityLabel->setText(quantities.join("\n"));
    ui->IDlabel->setText(idStrings.join("\n"));
}

void MainWindow::onResetPotCalcButtonClicked() {
    ui->potentialSellPriceSpinBox->setValue(0.0);
    ui->potentialCalculatedTaxLabel->clear();
    calculateTotalTaxBase();
}
