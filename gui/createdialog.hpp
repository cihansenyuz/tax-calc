#ifndef CREATEDIALOG_HPP
#define CREATEDIALOG_HPP

#include <QDialog>
#include "../inc/asset.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class Dialog; }
QT_END_NAMESPACE

class CreateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateDialog(QWidget *parent = nullptr);
    ~CreateDialog();

    QString symbol() const;
    QString name() const;
    double buyPrice() const;
    QDate buyDate() const;

signals:
    void assetCreated(const Asset &new_asset);

private slots:
    void onOkClicked();

private:
    Ui::Dialog *ui;
};

#endif // CREATEDIALOG_HPP
