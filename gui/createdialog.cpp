#include "createdialog.hpp"
#include "ui_createdialog.h"
#include <QMessageBox>

CreateDialog::CreateDialog(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::Dialog) {
    ui->setupUi(this);
    connect(ui->buttonBox, &QDialogButtonBox::accepted,
            this, &CreateDialog::onOkClicked);
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

    Asset newAsset = Asset::createWithUniqueId(symbolStr.toStdString(),
                                            nameStr.toStdString(),
                                            date,
                                            price,
                                            quantity
                                        );
    emit assetCreated(newAsset);
    accept();
}

CreateDialog::~CreateDialog(){ delete ui; }
QString CreateDialog::symbol() const { return ui->symbolLineEdit->text(); }
QString CreateDialog::name() const { return ui->nameLineEdit->text(); }
double CreateDialog::buyPrice() const { return ui->buyPriceSpinBox->value(); }
QDate CreateDialog::buyDate() const { return ui->buyDateEdit->date(); }