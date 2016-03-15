#include "myFinanceMainWindow.h"
#include "ui_myFinanceMainWindow.h"

#include "myDatabaseDatatype.h"

#include <QDebug>
#include <QMessageBox>
#include "myFinanceExchangeWindow.h"

myFinanceMainWindow::myFinanceMainWindow(myStockCodeName *inStockCode, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::myFinanceMainWindow), statusLabel(this),
    stockCode(inStockCode)
{
    assetModel = new myAssetModel(this);
    ui->setupUi(this);

    ui->treeView->setModel(assetModel);
    //ui->treeView->header()->resizeSections(QHeaderView::ResizeToContents);
    ui->treeView->header()->resizeSections(QHeaderView::Fixed);
    ui->treeView->header()->resizeSection(0, 170);
    ui->treeView->header()->resizeSection(1, 90);
    ui->treeView->header()->resizeSection(2, 50);
    ui->treeView->expandAll();

    connect(assetModel, SIGNAL(priceDataReflashed()), this, SLOT(priceDataReflashed()));
    connect(stockCode ,SIGNAL(codeDataReady()), this, SLOT(codeDataReady()));

    if (!stockCode->getIsDataReady()) { //正在更新
        ui->reflash->setEnabled(false);
        statusLabel.setText("正在读取股票代码");
        ui->statusBar->addWidget(&statusLabel);
    } else {
        ui->reflash->setEnabled(true);
        statusLabel.setText("ready");
    }
}

myFinanceMainWindow::~myFinanceMainWindow()
{
    delete ui;
    delete assetModel;
}
void myFinanceMainWindow::priceDataReflashed() {
    ui->treeView->header()->resizeSection(0, 170);
    ui->treeView->header()->resizeSection(1, 90);
    ui->treeView->header()->resizeSection(2, 50);

    ui->treeView->expandAll();
    ui->dateTimePrice->setDateTime(QDateTime::currentDateTime());
    qDebug() << "价格更新 finished";
}
void myFinanceMainWindow::codeDataReady() {
    if (stockCode->getIsDataReady()) {
        ui->reflash->setEnabled(true);
    }
    statusLabel.setText("ready");
}

void myFinanceMainWindow::on_exchange_clicked()
{
    qDebug() << "资产变化 clicked";
    if (!assetModel->getRootNode()) {
        QMessageBox::information(NULL, "提示", "rootNode is invalid");
        return;
    }
    myFinanceExchangeWindow exWin(stockCode, assetModel->getRootNode(), this);
    if(exWin.exec() == QDialog::Accepted) {
        assetModel->doExchange(exWin.getExchangeData());
        qDebug() << "资产变化 Accepted assetModel->doExchange finished";
    } else {
        qDebug() << "资产变化 Canceled";
    }

    ui->treeView->expandAll();
}

void myFinanceMainWindow::on_new_account_clicked()
{
    qDebug() << "新建帐户 clicked";
}

void myFinanceMainWindow::on_reflash_clicked()
{
    statusLabel.setText("正在读取股票代码");
    ui->reflash->setEnabled(false);
    qDebug() << "刷新 clicked";
    assetModel->doReflashAssetData();
    stockCode->getStockCode();
    ui->treeView->expandAll();
    qDebug() << "刷新 clicked finished";
}

void myFinanceMainWindow::on_updatePrice_clicked()
{
    qDebug() << "更新价格 clicked";
    assetModel->doUpdatePrice();
    ui->treeView->expandAll();
    qDebug() << "更新价格 clicked assetModel->doUpdatePrice requested";
}
