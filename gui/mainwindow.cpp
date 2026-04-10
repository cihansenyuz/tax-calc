#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "../inc/calculator.hpp"
#include "../inc/network/httpmanager.hpp"
#include "evdskeysdialog.hpp"
#include <QMessageBox>
#include <QSettings>
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

    connect(ui->actionEVDSKey, &QAction::triggered, this, [this]() {
        EvdsKeyDialog dialog(this);
        QSettings settings;
        dialog.setCurrentKey(settings.value("evds/api_key").toString());
        if (dialog.exec() == QDialog::Accepted) {
            QString newKey = dialog.key();
            settings.setValue("evds/api_key", newKey);
            HttpManager::getInstance()->setKey(newKey);
        }
    });

    // Drain loop setups starts here.
    m_closeDrain.connectAdvance = [this](std::function<void()> proceed) {
        return connect(transaction_manager, &TransactionManager::databaseReady,
                this, proceed, Qt::SingleShotConnection);
    };

    m_closeDrain.action = [this](const Transaction &t) {
        connect(transaction_manager, &TransactionManager::fetchFailed,
                this, &MainWindow::abortClose, Qt::SingleShotConnection);
        
        qDebug() << "Closing transaction with ID:" << t.getId();
        
        QTimer::singleShot(1000, this, [this, t]() {
                transaction_manager->closeTransaction(t);
        });
    };

    m_closeDrain.onDone = [this]() {
        onCleanSelectionButtonClicked();
        onResetPotCalcButtonClicked();
    };

    m_deleteDrain.connectAdvance = [this](std::function<void()> proceed) {
        return connect(transaction_manager, &TransactionManager::databaseReady,
                this, proceed, static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::SingleShotConnection));
    };

    m_deleteDrain.action = [this](const Transaction &t) {
        qDebug() << "Deleting transaction with ID:" << t.getId();
        try {
            transaction_manager->removeTransaction(t.getId());
        } catch (const std::runtime_error &e) {
            QObject::disconnect(m_deleteDrain.nextConnection);
            QMessageBox::warning(this, "Pozisyon Sil", e.what());
            m_deleteDrain.queue = {};
            onCleanSelectionButtonClicked();
        }
    };

    m_deleteDrain.onDone = [this]() {
        QMessageBox::information(this, "Pozisyon Sil", "Seçili pozisyonlar başarıyla silindi.");
        onCleanSelectionButtonClicked();
    };

    m_potentialDrain.connectAdvance = [this](std::function<void()> proceed) {
        return connect(transaction_manager, &TransactionManager::potentialTaxBaseReady,
                this, [this, proceed](double potentialTaxBase) {
                    m_potentialAccumulator += potentialTaxBase;
                    proceed();
                }, Qt::SingleShotConnection);
    };

    m_potentialDrain.action = [this](const Transaction &t) {
        connect(transaction_manager, &TransactionManager::fetchFailed,
                this, &MainWindow::abortPotentialCalc, Qt::SingleShotConnection);

        qDebug() << "Calculating potential tax base for ID:" << t.getId();
        
        QTimer::singleShot(1000, this, [this, t]() {
                transaction_manager->potentialTransaction(t);
        });
    };

    m_potentialDrain.onDone = [this]() {
        ui->potentialCalculatedTaxLabel->setText(QString::number(m_potentialAccumulator, 'f', 2) + " ₺");
        calculateTotalTaxBase(m_potentialAccumulator);
    };
    // Drain loop setups ends here.
    
    calculateTotalTaxBase();
}

MainWindow::~MainWindow() {
    if (m_closeDrain.progressDialog) {
        m_closeDrain.progressDialog->deleteLater();
        m_closeDrain.progressDialog = nullptr;
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

    m_deleteDrain.queue = {};
    for (const Transaction& t : m_selectedTransactions)
        m_deleteDrain.queue.push(t);

    runNext(m_deleteDrain);
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

    m_closeDrain.queue = {};
    for (Transaction t : m_selectedTransactions) {
        t.setSellDate(sellDate);
        t.setSellPrice(sellPrice);
        t.setStatus(Transaction::Status::Closed);
        m_closeDrain.queue.push(t);
    }

    m_closeDrain.total = static_cast<int>(m_closeDrain.queue.size());
    m_closeDrain.progressDialog = new QProgressDialog("Pozisyonlar kapatılıyor...", QString(), 0, m_closeDrain.total, this);
    m_closeDrain.progressDialog->setWindowModality(Qt::WindowModal);
    m_closeDrain.progressDialog->setMinimumDuration(0);
    m_closeDrain.progressDialog->setValue(0);

    runNext(m_closeDrain);
}

void MainWindow::abortClose(const QString &error) {
    QObject::disconnect(m_closeDrain.nextConnection);
    if (m_closeDrain.progressDialog) {
        m_closeDrain.progressDialog->deleteLater();
        m_closeDrain.progressDialog = nullptr;
    }
    QMessageBox::warning(this, "Pozisyon Kapat", error);
    m_closeDrain.queue = {};
    onCleanSelectionButtonClicked();
    qCritical(logNetwork) << "Failed to close transaction due to EVDS API error with ID:"
                          << m_closeDrain.pendingId << "Error:" << error;
}

void MainWindow::abortPotentialCalc(const QString &error) {
    QObject::disconnect(m_potentialDrain.nextConnection);
    if (m_potentialDrain.progressDialog) {
        m_potentialDrain.progressDialog->deleteLater();
        m_potentialDrain.progressDialog = nullptr;
    }
    QMessageBox::warning(this, "Potansiyel Hesaplama", error);
    m_potentialDrain.queue = {};
    m_potentialAccumulator = 0.0;
    qCritical(logNetwork) << "Failed potential calc due to EVDS API error with ID:"
                          << m_potentialDrain.pendingId << "Error:" << error;
}

void MainWindow::runNext(DrainContext &ctx) {
    if (ctx.queue.empty()) {
        if (ctx.progressDialog) {
            ctx.progressDialog->setValue(ctx.total);
            ctx.progressDialog->deleteLater();
            ctx.progressDialog = nullptr;
        }
        ctx.onDone();
        return;
    }

    if (ctx.progressDialog)
        ctx.progressDialog->setValue(ctx.total - static_cast<int>(ctx.queue.size()));

    Transaction t = ctx.queue.front();
    ctx.queue.pop();
    ctx.pendingId = t.getId();

    ctx.nextConnection = ctx.connectAdvance([this, &ctx]() { runNext(ctx); });

    ctx.action(t);
}

void MainWindow::onPotentialCalculateButtonClicked() {
    if (m_selectedTransactions.empty()) {
        QMessageBox::warning(this, "Seçim Hatası", "Potansiyel vergi hesaplamak için lütfen bir pozisyon seçin.");
        return;
    }

    double potentialSellPrice = ui->potentialSellPriceSpinBox->value();
    if (potentialSellPrice <= 0) {
        QMessageBox::warning(this, "Giriş Hatası", "Lütfen geçerli bir satış fiyatı girin.");
        return;
    }

    QDate currentDate = QDate::currentDate();
    m_potentialDrain.queue = {};
    for (Transaction t : m_selectedTransactions) {
        t.setSellDate(currentDate);
        t.setSellPrice(potentialSellPrice);
        m_potentialDrain.queue.push(t);
    }

    m_potentialDrain.total = static_cast<int>(m_potentialDrain.queue.size());
    m_potentialDrain.progressDialog = new QProgressDialog("Potansiyel gelir ve vergisi hesaplanıyor...", QString(), 0, m_potentialDrain.total, this);
    m_potentialDrain.progressDialog->setWindowModality(Qt::WindowModal);
    m_potentialDrain.progressDialog->setMinimumDuration(0);
    m_potentialDrain.progressDialog->setValue(0);

    m_potentialAccumulator = 0.0;
    runNext(m_potentialDrain);
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
