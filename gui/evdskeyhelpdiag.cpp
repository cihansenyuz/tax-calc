#include "evdskeyhelpdiag.hpp"
#include "ui_evdskeyhelpdiag.h"

EvdsKeyHelpDialog::EvdsKeyHelpDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::EvdsKeyHelpDialog) {
    ui->setupUi(this);
}

EvdsKeyHelpDialog::~EvdsKeyHelpDialog() {
    delete ui;
}

void EvdsKeyHelpDialog::setText(const QString &text) {
    ui->labelContent->setText(text);
}
