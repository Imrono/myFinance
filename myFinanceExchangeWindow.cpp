#include "myFinanceExchangeWindow.h"
#include "ui_myFinanceExchangeWindow.h"

#include <QtCore/QMap>

#include <QMessageBox>
#include <QtDebug>

myFinanceExchangeWindow::myFinanceExchangeWindow(myStockCodeName *inStockCode, myAssetNode* rootNode, QWidget *parent) :
    QDialog(parent), grpBuySell(nullptr),
    ui(new Ui::myFinanceExchangeWindow),
    stockCode(inStockCode)
{
    ui->setupUi(this);

    grpBuySell = new QButtonGroup(this);
    grpBuySell->addButton(ui->radioBuy);
    grpBuySell->addButton(ui->radioSell);
    grpBuySell->setExclusive(true);         //设为互斥
    grpBuySell->setId(ui->radioBuy,  BUY);  //radioBuy的Id设为0
    grpBuySell->setId(ui->radioSell, SELL); //radioBuy的Id设为1
    ui->radioBuy->setChecked(true);

    grpMarket = new QButtonGroup(this);
    grpMarket->addButton(ui->radioSH);
    grpMarket->addButton(ui->radioSZ);
    grpMarket->addButton(ui->radioOther);
    grpMarket->setExclusive(true);           //设为互斥
    grpMarket->setId(ui->radioSH, SH);       //radioBuy的Id设为0
    grpMarket->setId(ui->radioSZ, SZ);       //radioBuy的Id设为1
    grpMarket->setId(ui->radioOther, OTHER); //radioBuy的Id设为2
    ui->radioSH->setChecked(true);
    updateMarketInfo();

    dataSource = 0;
    ui->tabWidget->setCurrentIndex(dataSource);
    updataData();
    updateBuySell();

    initial(rootNode);

    data.money  = 0.0f;
    data.price  = 0.0f;
    data.amount = 0;
}

myFinanceExchangeWindow::~myFinanceExchangeWindow()
{
    if (!grpMarket)
        delete grpMarket;
    if (!grpBuySell)
        delete grpBuySell;
    delete ui;
}

void myFinanceExchangeWindow::initial(myAssetNode* rootNode) {
    ui->moneyAccount->clear();
    // 交易：资产变化
    for (int i = 0; i < rootNode->children.count(); i++) {
        myAssetNode *accountNode = rootNode->children.at(i);
        QIcon   icon = QString("resource//icon//%1").arg(accountNode->nodeData.value<myAssetAccount>().logo);
        QString code = accountNode->nodeData.value<myAssetAccount>().code;
        ui->moneyAccount->addItem(icon, code);
    }
    // 转帐：资金变化
    for (int i = 0; i < rootNode->children.count(); i++) {
        myAssetNode *accountNode = rootNode->children.at(i);
        for (int j = 0; j < accountNode->children.count(); j++) {
            myAssetNode *holdNode = accountNode->children.at(j);
            QString assetCode = holdNode->nodeData.value<myAssetHold>().assetCode;
            if (assetCode.contains("cash")) {
                QIcon   icon = QString("resource//icon//%1").arg(accountNode->nodeData.value<myAssetAccount>().logo);
                QString code = accountNode->nodeData.value<myAssetAccount>().code;
                ui->moneyAccountOut->addItem(icon, code);
                ui->moneyAccountIn->addItem(icon, code);
                break;
            }
        }
    }
}

void myFinanceExchangeWindow::on_buttonBox_accepted()
{
    //QMessageBox::information(this,"提示","这是一个消息框 new_account");
    data.time     = ui->timeDateTimeEdit->dateTime();
    data.type     = ui->typeLineEdit->text();
    updataData();

    /// test
    qDebug() << data.time;
    qDebug() << data.type;
    qDebug() << data.account1;
    qDebug() << data.money;
    qDebug() << data.account2;
    qDebug() << data.code;
    qDebug() << data.name;
    qDebug() << data.amount;
    qDebug() << data.price;
}

void myFinanceExchangeWindow::on_amountSpinBox_valueChanged(int value)
{
    data.amount = value;
    updateBuySell();
}
void myFinanceExchangeWindow::on_priceSpinBox_valueChanged(double value) {
    data.price = value;
    updateBuySell();
}
void myFinanceExchangeWindow::on_radioBuy_clicked() {
    updateBuySell();
}
void myFinanceExchangeWindow::on_radioSell_clicked() {
    updateBuySell();
}
void myFinanceExchangeWindow::on_exchangeFeeSpinBox_valueChanged(double value) {
    data.fee = value;
    updateBuySell();
}
void myFinanceExchangeWindow::updateBuySell() {
    data.buySell = grpBuySell->checkedId() == BUY ? (bool)BUY : (bool)SELL;
    buySellFlag = data.buySell == SELL ? 1.0f : -1.0f;

	data.amount = ui->amountSpinBox->text().toInt();
	data.amount *= -buySellFlag;
    data.money = -(float)data.amount * data.price - data.fee;

    ui->moneySpinBox->setValue(data.money);
}


void myFinanceExchangeWindow::on_tabWidget_currentChanged(int index)
{
    dataSource = index;
    qDebug() << dataSource;
    updataData();
}

void myFinanceExchangeWindow::on_moneyTransferSpinBox_valueChanged(double value)
{
    data.price = value;
    data.money = -data.price;
}

void myFinanceExchangeWindow::updataData() {
    if (0 == dataSource) {
        data.account1 = ui->moneyAccount->itemText(ui->moneyAccount->currentIndex());
        data.account2 = data.account1;
        data.amount = ui->amountSpinBox->text().toInt() * -buySellFlag;
        data.price  = ui->priceSpinBox->text().toDouble();
        data.money  = ui->moneySpinBox->text().toDouble();
        data.code   = ui->codeLineEdit->text();
        data.name   = ui->nameLineEdit->text();
        data.fee    = ui->exchangeFeeSpinBox->text().toDouble();

    } else if (1 == dataSource) {
        data.price    = ui->moneyTransferSpinBox->text().toDouble();

        data.account1 = ui->moneyAccountOut->itemText(ui->moneyAccountOut->currentIndex());
        data.money    = -data.price;

        data.account2 = ui->moneyAccountIn->itemText(ui->moneyAccountIn->currentIndex());
        data.code     = CASH;
        data.name     = QString::fromLocal8Bit("现有资金");
        data.amount   = 1;
        data.fee      = 0.0f;
    } else {}
}

void myFinanceExchangeWindow::on_radioSH_clicked() {
    updateMarketInfo();
}

void myFinanceExchangeWindow::on_radioSZ_clicked() {
    updateMarketInfo();
}

void myFinanceExchangeWindow::on_radioOther_clicked() {
    updateMarketInfo();
}

void myFinanceExchangeWindow::updateMarketInfo() {
    QString market;
    switch (grpMarket->checkedId()) {
    case SH:
        market = "sh.";
        break;
    case SZ:
        market = "sz.";
        break;
    case OTHER:
        break;
    default:
        break;
    }
    int pointIndex = data.code.indexOf(QString("."));
    data.code.remove(0, pointIndex+1);
    data.code.insert(0, market);
    ui->codeLineEdit->setText(data.code);
}

void myFinanceExchangeWindow::on_codeLineEdit_textChanged(const QString &str)
{
    data.code = str;
    updateMarketInfo();
}

void myFinanceExchangeWindow::on_codeLineEdit_editingFinished()
{
    int count = stockCode->codeName.count();
    qDebug() << count;
    if (stockCode->getIsInitialed()) {
        if (stockCode->codeName.contains(data.code)) {
            QMap<QString, QString>::const_iterator ii = stockCode->codeName.find(data.code);
            if (ii != stockCode->codeName.end() && ii.key() == data.code) {
                data.name = ii.value();
                ui->nameLineEdit->setText(data.name);
            }
        }
    }
}
