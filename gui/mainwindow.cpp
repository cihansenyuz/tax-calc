#include "mainwindow.hpp"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_asset_manager = new AssetManager(this);
    connect(ui->createButton, &QPushButton::clicked, this, &MainWindow::onCreateButtonClicked);
    connect(ui->printButton, &QPushButton::clicked, this, &MainWindow::onPrintButtonClicked);
}

MainWindow::~MainWindow()
{
    delete m_asset_manager;
    delete ui;
}

void MainWindow::onCreateButtonClicked()
{
    if (!m_create_dialog) {
        m_create_dialog = std::make_unique<CreateDialog>(this);
        connect(m_create_dialog.get(), &CreateDialog::assetCreated, this,
                [this](const Asset &new_asset) {
            m_asset_manager->addAsset(new_asset);
        });
        m_create_dialog->exec();
        m_create_dialog.reset();
    }
}

void MainWindow::onPrintButtonClicked()
{
    m_asset_manager->printAssets();
    qDebug() << "Total assets:" << m_asset_manager->size();
}