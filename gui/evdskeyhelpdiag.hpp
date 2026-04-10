#pragma once

#include <QDialog>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui { class EvdsKeyHelpDialog; }
QT_END_NAMESPACE

class EvdsKeyHelpDialog : public QDialog {
    Q_OBJECT

public:
    explicit EvdsKeyHelpDialog(QWidget *parent = nullptr);
    ~EvdsKeyHelpDialog();

    void setText(const QString &text);

private:
    Ui::EvdsKeyHelpDialog *ui;
};
