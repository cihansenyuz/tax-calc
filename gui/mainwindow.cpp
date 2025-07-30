#include "mainwindow.hpp"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_asset_manager = new AssetManager(this);
    connect(ui->createButton, &QPushButton::clicked, this, &MainWindow::onCreateButtonClicked);
    connect(ui->closeTransactionButton, &QPushButton::clicked, this, &MainWindow::onCloseTransactionButtonClicked);
    
    qobject_cast<QVBoxLayout*>(ui->centralwidget->layout())->insertWidget(0, &m_table);
    m_table.refresh(m_asset_manager->getAssets());
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
        connect(m_create_dialog.get(), &CreateDialog::assetCreated,
                m_asset_manager, &AssetManager::openTransaction, Qt::SingleShotConnection);
        connect(m_asset_manager, &AssetManager::databaseReady,
                this, [this](){ 
                    qDebug() << "Database is ready, refreshing table.";
                    m_table.refresh(m_asset_manager->getAssets()); }, Qt::SingleShotConnection);

        m_create_dialog->exec();
        m_create_dialog.reset();
    }
}

void MainWindow::onCloseTransactionButtonClicked(){
    int selectedRow = m_table.currentRow();
    QTableWidgetItem *item = m_table.item(selectedRow, 8);

    Asset selectedAsset = m_asset_manager->findAssetById(item->text().toInt());

    m_asset_manager->closeTransaction(selectedAsset);
    connect(m_asset_manager, &AssetManager::databaseReady,
                this, [this](){ 
                    qDebug() << "Database is ready, refreshing table.";
                    m_table.refresh(m_asset_manager->getAssets()); }, Qt::SingleShotConnection);
}