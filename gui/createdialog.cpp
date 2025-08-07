#include "createdialog.hpp"
#include "ui_createdialog.h"
#include <QMessageBox>

CreateDialog::CreateDialog(QWidget *parent)
    : QDialog(parent), fetcher(new YahooFinanceFetcher(this)),
      ui(new Ui::Dialog) {
    ui->setupUi(this);
    connect(ui->buttonBox, &QDialogButtonBox::accepted,
            this, &CreateDialog::onOkClicked);
    
    connect(fetcher, &YahooFinanceFetcher::symbolsFetched,
            this, [this](const QList<QPair<QString, QString>> &symbols) {
                for (const auto &pair : symbols) {
                    qDebug() << pair.first << " - " << pair.second;
                }
    });

    searchTimer = new QTimer(this);
    searchTimer->setInterval(750); // 750 ms delay for search
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

CreateDialog::~CreateDialog(){ delete ui; delete fetcher; delete searchTimer; }
QString CreateDialog::symbol() const { return ui->symbolLineEdit->text(); }
QString CreateDialog::name() const { return ui->nameLineEdit->text(); }
double CreateDialog::buyPrice() const { return ui->buyPriceSpinBox->value(); }
QDate CreateDialog::buyDate() const { return ui->buyDateEdit->date(); }