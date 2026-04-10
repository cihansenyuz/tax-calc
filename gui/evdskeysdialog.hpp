#pragma once

#include <QDialog>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui { class EvdsKeyDialog; }
QT_END_NAMESPACE

class EvdsKeyDialog : public QDialog {
    Q_OBJECT

public:
    explicit EvdsKeyDialog(QWidget *parent = nullptr);
    ~EvdsKeyDialog();

    void setCurrentKey(const QString &key);
    QString key() const;

private:
    Ui::EvdsKeyDialog *ui;
};
