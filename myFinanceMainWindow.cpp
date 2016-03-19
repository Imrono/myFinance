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
    exchangeModel = new myExchangeListModel();

    ui->setupUi(this);

    ui->treeView->setModel(assetModel);
    //ui->treeView->header()->resizeSections(QHeaderView::ResizeToContents);
    ui->treeView->header()->resizeSections(QHeaderView::Fixed);
    ui->treeView->header()->resizeSection(0, 170);
    ui->treeView->header()->resizeSection(1, 90);
    ui->treeView->header()->resizeSection(2, 50);
    ui->treeView->expandAll();

    ui->listView->setModel(exchangeModel);

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
    delete exchangeModel;
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
    myFinanceExchangeWindow exWin(this);
    if(exWin.exec() == QDialog::Accepted) {
        exchangeData data = exWin.getExchangeData();
        exchangeAbnomal abnormalCode = NORMAL;
        if (assetModel->checkExchange(data, abnormalCode)) {
            assetModel->doExchange(data);
            exchangeModel->doExchange(data);
            qDebug() << "资产变化 Accepted assetModel->doExchange finished";
        } else {
            QString info;
            if (LACK_MONEY_1 == abnormalCode) {
                info = data.account1 + QString::fromLocal8Bit(" Lack of Money 需要资金 ") + data.money + QString::fromLocal8Bit(" 但只存在");
            } else if (LACK_MONEY_2 == abnormalCode) {
                info = data.account2 + QString::fromLocal8Bit(" Lack of Money 需要资金 ") + data.price + QString::fromLocal8Bit(" 但只存在");
            } else if (LACK_STOCK == abnormalCode) {
                info = data.account2 + QString::fromLocal8Bit(" Lack of Stock 需要股票 ") + data.amount + QString::fromLocal8Bit(" 但只存在");
            } else if (LACK_STOCK == abnormalCode) {
                info = data.account1 + QString::fromLocal8Bit(" cash 返回不是一条结果");
            } else if (UN_UNIQUE_2 == abnormalCode) {
                info = data.account2 + " " + data.name + QString::fromLocal8Bit("返回不是一条结果");
            } else if (SQL_ERROR == abnormalCode) {
                info = QString::fromLocal8Bit("SQL错误");
            } else {
                info = QString::fromLocal8Bit("其它错误");
            }

            QMessageBox::information(NULL, QString::fromLocal8Bit("提示"), info);
            qDebug() << QString::fromLocal8Bit("资产变化 Accepted assetModel->checkExchange failure");
        }
    } else {
        qDebug() << QString::fromLocal8Bit("资产变化 Canceled");
    }

    ui->treeView->expandAll();
}

void myFinanceMainWindow::on_new_account_clicked()
{
    qDebug() << "新建帐户 clicked";
    exchangeModel->test();
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
