#include "evdskeysdialog.hpp"
#include "ui_evdskeysdialog.h"

EvdsKeyDialog::EvdsKeyDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::EvdsKeyDialog) {
    ui->setupUi(this);
}

EvdsKeyDialog::~EvdsKeyDialog() {
    delete ui;
}

void EvdsKeyDialog::setCurrentKey(const QString &key) {
    ui->lineEditApiKey->setText(key);
}

QString EvdsKeyDialog::key() const {
    return ui->lineEditApiKey->text().trimmed();
}
