#include "createdialog.hpp"
#include "ui_createdialog.h"

CreateDialog::CreateDialog(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::Dialog)
{
    ui->setupUi(this);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &CreateDialog::onOkClicked);
}
void CreateDialog::onOkClicked()
{
    // Gather user input
    QString symbolStr = symbol();
    QString nameStr = name();
    double price = buyPrice();
    QDate date = buyDate();

    Asset newAsset(symbolStr.toStdString(),
                   nameStr.toStdString(),
                   date,
                   price);
    emit assetCreated(newAsset);
    accept();
}

CreateDialog::~CreateDialog()
{
    delete ui;
}

QString CreateDialog::symbol() const
{
    return ui->symbolLineEdit->text();
}

QString CreateDialog::name() const
{
    return ui->nameLineEdit->text();
}

double CreateDialog::buyPrice() const
{
    return ui->buyPriceSpinBox->value();
}

QDate CreateDialog::buyDate() const
{
    return ui->buyDateEdit->date();
}
