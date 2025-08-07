#include "createdialog.hpp"
#include "ui_createdialog.h"
#include <QMessageBox>
#include <QStringListModel>

CreateDialog::CreateDialog(QWidget *parent)
    : QDialog(parent), fetcher(new YahooFinanceFetcher(this)),
      ui(new Ui::Dialog) {
    ui->setupUi(this);
    connect(ui->buttonBox, &QDialogButtonBox::accepted,
            this, &CreateDialog::onOkClicked);
    
    symbolAndNameCompleter = new QCompleter(this);
    symbolAndNameCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    symbolAndNameCompleter->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    ui->nameLineEdit->setCompleter(symbolAndNameCompleter);

    connect(symbolAndNameCompleter, QOverload<const QString &>::of(&QCompleter::activated),
        this, [this](const QString &selection) {
            QMetaObject::invokeMethod(this, "onAssetSelected", Qt::QueuedConnection, Q_ARG(QString, selection));
        });

    connect(fetcher, &YahooFinanceFetcher::symbolsFetched,
            this, [this](const QList<QPair<QString, QString>> &symbols) {
                QStringList assetList;
                for (const auto &pair : symbols) {
                    assetList << pair.first + " | " + pair.second;
                }
                symbolAndNameCompleter->setModel(new QStringListModel(assetList, symbolAndNameCompleter));
                ui->nameLineEdit->completer()->complete();
    });

    searchTimer = new QTimer(this);
    searchTimer->setInterval(500); // 500 ms delay for search
    connect(searchTimer, &QTimer::timeout, this, [this]() {
        QString query = ui->nameLineEdit->text();
        if (!query.isEmpty()) {
            fetcher->fetchSymbols(query);
            searchTimer->stop();
        }
    });

    connect(ui->nameLineEdit, &QLineEdit::textEdited,
        this, [this]() {
            qDebug() << "Name field edited, restarting search timer.";
            searchTimer->start(); // Restarts the timer on each keystroke
    });
}

void CreateDialog::onAssetSelected(const QString &selection) {
    QStringList parts = selection.split(" | ");
    if (parts.size() == 2) {
        ui->nameLineEdit->setText(parts[1]);
        qDebug() << "Selected asset name:" << parts[1];
        ui->symbolLineEdit->setText(parts[0]);
    }
}

void CreateDialog::onOkClicked() {
    QString symbolStr = symbol();
    QString nameStr = name();
    double price = buyPrice();
    QDate date = buyDate();
    int quantity = ui->quantitySpinBox->value();

    if (symbolStr.isEmpty() || nameStr.isEmpty() || price <= 0 || quantity <= 0) {
        QMessageBox::warning(this, "Giriş Hatası", "Lütfen tüm alanları doğru şekilde doldurun.");
        return;
    }

    Transaction newTransaction = Transaction::createWithUniqueId(symbolStr.toStdString(),
                                            nameStr.toStdString(),
                                            date,
                                            price,
                                            quantity
                                        );
    emit assetCreated(newTransaction);
    accept();
}

CreateDialog::~CreateDialog(){ delete ui; delete fetcher; delete searchTimer; delete symbolAndNameCompleter; }
QString CreateDialog::symbol() const { return ui->symbolLineEdit->text(); }
QString CreateDialog::name() const { return ui->nameLineEdit->text(); }
double CreateDialog::buyPrice() const { return ui->buyPriceSpinBox->value(); }
QDate CreateDialog::buyDate() const { return ui->buyDateEdit->date(); }