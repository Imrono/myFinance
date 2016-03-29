#include "myFinanceExchangeWindow.h"
#include "ui_myFinanceExchangeWindow.h"

#include <QtCore/QMap>
#include "myFinanceMainWindow.h"

#include <QtDebug>

myFinanceExchangeWindow::myFinanceExchangeWindow(QWidget *parent) :
    QDialog(parent), grpBuySell(nullptr),
    ui(new Ui::myFinanceExchangeWindow),
    stockCode(myStockCodeName::getInstance())
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

    initial(static_cast<myFinanceMainWindow *>(parent)->getAssetModel()->getRootNode());

    data.money  = 0.0f;
    data.price  = 0.0f;
    data.amount = 0;
    commisionRate = 0.0f;
    updateExchangeFee();
}

myFinanceExchangeWindow::~myFinanceExchangeWindow()
{
    if (!grpMarket)
        delete grpMarket;
    if (!grpBuySell)
        delete grpBuySell;
    delete ui;
}

void myFinanceExchangeWindow::initial(const myRootAccountAsset &rootNode) {
    ui->moneyAccount->clear();
    // 交易：资产变化
    for (int i = 0; i < rootNode.getAccountCount(); i++) {
        myAssetNode *accountNode = rootNode.getAccountNode(i);
        QIcon   icon = QString("resource//icon//%1").arg(accountNode->nodeData.value<myAssetAccount>().logo);
        QString code = accountNode->nodeData.value<myAssetAccount>().code;
        ui->moneyAccount->addItem(icon, code);
    }
    // 转帐：资金变化
    for (int i = 0; i < rootNode.getAccountCount(); i++) {
        myAssetNode *accountNode = rootNode.getAccountNode(i);
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
    qDebug() << QString::fromLocal8Bit("资产变化 accepted");
    data.time     = ui->timeDateTimeEdit->dateTime();
    data.type     = ui->typeLineEdit->text();
    updataData();
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
    qDebug() << "radioBuy clicked";
    updateBuySell();
}
void myFinanceExchangeWindow::on_radioSell_clicked() {
    qDebug() << "radioSell clicked";
    updateBuySell();
}
void myFinanceExchangeWindow::on_exchangeFeeSpinBox_valueChanged(double value) {
    qDebug() << "exchangeFeeSpinBox valueChanged";
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

    updateExchangeFee();
    updateExchangeType();

    qDebug() << "data.buySell " << (grpBuySell->checkedId() == BUY ? "BUY" : "SELL") << ","
             << "data.amount " << data.amount << ","
             << "data.money "  << data.money  << ",";
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
        if (grpBuySell->checkedId() == BUY) {
            data.type = QString::fromLocal8Bit("证券买入");
        } else {
            data.type = QString::fromLocal8Bit("证券卖出");
        }

        data.account1 = ui->moneyAccount->itemText(ui->moneyAccount->currentIndex());
        data.account2 = data.account1;
        data.amount = ui->amountSpinBox->text().toInt() * -buySellFlag;
        data.price  = ui->priceSpinBox->text().toDouble();
        data.money  = ui->moneySpinBox->text().toDouble();
        data.code   = ui->codeLineEdit->text();
        data.name   = ui->nameLineEdit->text();
        data.fee    = ui->exchangeFeeSpinBox->text().toDouble();

    } else if (1 == dataSource) {
        data.type = QString::fromLocal8Bit("转帐");

        data.account1 = ui->moneyAccountOut->itemText(ui->moneyAccountOut->currentIndex());
        data.money    = -ui->moneyTransferSpinBox->text().toDouble();

        data.account2 = ui->moneyAccountIn->itemText(ui->moneyAccountIn->currentIndex());
        data.code     = CASH;
        data.name     = QString::fromLocal8Bit("现有资金");
        data.amount   = 1;
        data.price    = ui->moneyTransferSpinBox->text().toDouble();
        data.fee      = 0.0f;
    } else {}
    updateExchangeType();

    qDebug() << "data.time "     << data.time << ","
             << "data.type "     << data.type << ","
             << "data.account1 " << data.account1 << ","
             << "data.money "    << data.money;
    qDebug() << "data.account2 " << data.account2 << ","
             << "data.code "     << data.code << ","
             << "data.name "     << data.name << ","
             << "data.amount "   << data.amount << ","
             << "data.price "    << data.price;
}

void myFinanceExchangeWindow::on_radioSH_clicked() {
    qDebug() << "radioSH clicked";
    updateMarketInfo();
}

void myFinanceExchangeWindow::on_radioSZ_clicked() {
    qDebug() << "radioSZ clicked";
    updateMarketInfo();
}

void myFinanceExchangeWindow::on_radioOther_clicked() {
    qDebug() << "radioOther clicked";
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
        market = "";
        break;
    default:
        break;
    }
    int pointIndex = data.code.indexOf(QString("."));
    data.code.remove(0, pointIndex+1);
    data.code.insert(0, market);
    if (data.code == "sh.sh") {
        data.code = "sh.";
    } else if (data.code == "sz.sz") {
        data.code = "sz.";
    } else {}
    ui->codeLineEdit->setText(data.code);

    updateExchangeFee();

    qDebug() << "updateMarketInfo()" << ","
             << "data.code"  << data.code << ","
             << (grpMarket->checkedId() == SH ? "radioSH" :
                 grpMarket->checkedId() == SZ ? "radioSZ" :
                 grpMarket->checkedId() == OTHER ? "radioOther" : "radioUnknown");
}

void myFinanceExchangeWindow::on_codeLineEdit_textChanged(const QString &str)
{
    data.code = str;

    // 上海，深圳通过股票代码自动判断
    int pointIndex = data.code.indexOf(QString("."));
    int len = data.code.size();
    if (len - pointIndex > 2 &&
        (grpMarket->checkedId() == SH ||
         grpMarket->checkedId() == SZ)) {
        QString subStr = data.code.mid(pointIndex+1, 2);
        if (subStr == "30" || subStr == "00") {
            ui->radioSZ->setChecked(true);
        } else if (subStr == "60") {
            ui->radioSH->setChecked(true);
        } else {
            subStr = data.code.mid(pointIndex+1, 4);
            if (subStr == "cash") {
                ui->radioOther->setChecked(true);
            } else {}
        }
    }

    updateMarketInfo();
}

void myFinanceExchangeWindow::on_codeLineEdit_editingFinished()
{
    int count = stockCode->codeName.count();
    qDebug() << QString::fromLocal8Bit("代号EditLine") << ui->codeLineEdit->text() << "(" << count << ")";
    if (OTHER != grpMarket->checkedId()) {
        if (stockCode->getIsInitialed()) {
            if (stockCode->codeName.contains(data.code)) {
                QMap<QString, QString>::const_iterator ii = stockCode->codeName.find(data.code);
                if (ii != stockCode->codeName.end() && ii.key() == data.code) {
                    data.name = ii.value();
                    ui->nameLineEdit->setText(data.name);
                }
            } else {
                data.name = "";
                ui->nameLineEdit->setText(data.name);
            }
        }
    }
}

void myFinanceExchangeWindow::on_nameLineEdit_editingFinished()
{
    QString str = ui->nameLineEdit->text();
    int count = stockCode->codeName.count();
    qDebug() << QString::fromLocal8Bit("名称EditLine") << str << "(" << count << ")";

    QMap<QString,QString>::iterator it = stockCode->codeName.begin();
    for (; it != stockCode->codeName.end(); ++it) {
        if (it.value() == str) {
            data.code = it.key();
            QRegExp rx("([a-zA-Z]*)[.][0-9]*");
            int pos = data.code.indexOf(rx);
            if (pos >= 0) {
                QString a = rx.cap(1);
                if (a == "sh") {
                    ui->radioSH->setChecked(true);
                } else if (a == "sz") {
                    ui->radioSZ->setChecked(true);
                } else {
                    ui->radioOther->setChecked(true);
                }
                updateMarketInfo();
            }

            ui->codeLineEdit->setText(data.code);
            qDebug() << data.code << it.value();
            return;
        }
    }

    ui->radioOther->setChecked(true);
    updateMarketInfo();
}

void myFinanceExchangeWindow::on_feeRateSpinBox_valueChanged(double feeRate)
{
    qDebug() << QString::fromLocal8Bit("佣金") << feeRate;
    commisionRate = feeRate * 0.001f;
    updateExchangeFee();
}
void myFinanceExchangeWindow::updateExchangeFee() {
    double fee = 0.0f;
    double amount = qAbs(static_cast<double>(data.amount));
    double fee1 = 0.0f; //佣金
    fee1 = data.price * amount * commisionRate;
    if (fee1 < 5.0f) {
        fee1 = 5.0f;
    } else {}

    double fee2 = 0.0f; //过户费
    if (SH == grpMarket->checkedId()) {
        fee2 = data.price * amount * 0.02f*0.001f;  //0.02‰
    } else {}

    double fee3 = 0.0f; //印花税
    if (SELL == grpBuySell->checkedId()) {
        fee3 = data.price * amount * 0.001f;
    } else {}

    fee = fee1 + fee2 + fee3;
    ui->exchangeFeeSpinBox->setValue(fee);
}

void myFinanceExchangeWindow::updateExchangeType() {
    if (0 == dataSource) {
        if (grpBuySell->checkedId() == BUY) {
            data.type = QString::fromLocal8Bit("证券买入");
        } else {
            data.type = QString::fromLocal8Bit("证券卖出");
        }
    } else if (1 == dataSource) {
        data.type = QString::fromLocal8Bit("转帐");
    } else {}
    ui->typeLineEdit->setText(data.type);
}
