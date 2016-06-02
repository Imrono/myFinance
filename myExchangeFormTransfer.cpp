#include "myExchangeFormTransfer.h"
#include "ui_myExchangeFormTransfer.h"

myExchangeFormTransfer::myExchangeFormTransfer(const myRootAccountAsset *rootNode, QString tabName, QWidget *parent) :
    myExchangeFormTabBase(rootNode, tabName, parent),
    ui(new Ui::myExchangeFormTransfer)
{
    ui->setupUi(this);

    data.code     = MY_CASH;
    data.amount   = 1;

    inIdx2AccountIdx.clear();
    outIdx2AccountIdx.clear();
    int localCount = 0;
    for (int i = 0; i < rootNode->getAccountCount(); i++) {
        myAssetNode *accountNode = rootNode->getAccountNode(i);
        for (int j = 0; j < accountNode->children.count(); j++) {
            myAssetNode *holdNode = accountNode->children.at(j);
            QString assetCode = holdNode->nodeData.value<myAssetHold>().assetCode;
            if (assetCode.contains("cash")) {
                const myAssetAccount &accountData = accountNode->nodeData.value<myAssetAccount>();
                QIcon   icon =QIcon( QString(":/icon/finance/resource/icon/finance/%1").arg(accountData.logo));
                QString code;
                if (accountData.name.contains(STR("ÒøÐÐ"))) {
                    code = "**** **** " + accountData.code.right(4);
                } else {
                    code = accountData.code;
                }
                inIdx2AccountIdx.insert(localCount, i);
                outIdx2AccountIdx.insert(localCount, i);
                ui->moneyAccountOut->addItem(icon, code);
                ui->moneyAccountIn->addItem(icon, code);
                localCount++;
                break;
            }
        }
    }
}

myExchangeFormTransfer::~myExchangeFormTransfer()
{
    delete ui;
}

void myExchangeFormTransfer::on_moneyAccountOut_currentIndexChanged(int index) {
    int nodeIdx = outIdx2AccountIdx.find(index).value();
    // 1. data.account1 & data.account2 update
    myAssetNode *accountNode = rootNode->getAccountNode(nodeIdx);
    const myAssetAccount accountData = accountNode->nodeData.value<myAssetAccount>();
    data.account1 = accountData.code;
    for (int j = 0; j < accountNode->children.count(); j++) {
        myAssetNode *holdNode = accountNode->children.at(j);
        QString assetCode = holdNode->nodeData.value<myAssetHold>().assetCode;
        if (assetCode.contains("cash")) {
            data.name = holdNode->nodeData.value<myAssetHold>().name;
            break;
        }
    }

    totalMoneyOut = getTotalMoney(nodeIdx);
    if (isModifyExchange) {
        totalMoneyOut -= data.money;
    }
    ui->moneySpinBoxTotalOut->setValue(totalMoneyOut);
    remainMoneyOut = totalMoneyOut + data.money;
    ui->moneySpinBoxRemainOut->setValue(remainMoneyOut);
    qDebug() << "#moneyAccountOut_currentIndexChanged# isModifyExchange:" << isModifyExchange
             << " totalMoneyOut:" << totalMoneyOut << " remainMoneyOut:" << remainMoneyOut;
}
void myExchangeFormTransfer::on_moneyAccountIn_currentIndexChanged(int index) {
    int nodeIdx = inIdx2AccountIdx.find(index).value();
    // 1. data.account1 & data.account2 update
    myAssetNode *accountNode = rootNode->getAccountNode(nodeIdx);
    const myAssetAccount accountData = accountNode->nodeData.value<myAssetAccount>();
    data.account2 = accountData.code;

    totalMoneyIn = getTotalMoney(nodeIdx);
    if (isModifyExchange) {
        totalMoneyIn -= data.money;
    }
    ui->moneySpinBoxTotalIn->setValue(totalMoneyIn);
    remainMoneyIn = totalMoneyIn - data.money;
    ui->moneySpinBoxRemainIn->setValue(remainMoneyIn);
    qDebug() << "#moneyAccountIn_currentIndexChanged# isModifyExchange:" << isModifyExchange
             << " totalMoneyIn:" << totalMoneyIn << " remainMoneyIn:" << remainMoneyIn;
}

void myExchangeFormTransfer::on_moneyTransferSpinBox_valueChanged(double value) {
    data.price = value;
    data.money = -data.price;

    remainMoneyOut = totalMoneyOut + data.money;
    ui->moneySpinBoxRemainOut->setValue(remainMoneyOut);
    remainMoneyIn = totalMoneyIn - data.money;
    ui->moneySpinBoxRemainIn->setValue(remainMoneyIn);
}

void myExchangeFormTransfer::recordExchangeData(myExchangeData &tmpData) {
    myExchangeFormTabBase::recordExchangeData(tmpData);

    tmpData.account1 = ui->moneyAccountOut->itemText(ui->moneyAccountOut->currentIndex());
    tmpData.money    = -ui->moneyTransferSpinBox->value() + data.fee;

    tmpData.account2 = ui->moneyAccountIn->itemText(ui->moneyAccountIn->currentIndex());
    tmpData.code     = MY_CASH;
    tmpData.name     = data.name;
    tmpData.amount   = 1;
    tmpData.price    = ui->moneyTransferSpinBox->value();
}
void myExchangeFormTransfer::setUI(const myExchangeData &exchangeData) {
    int indexOut = ui->moneyAccountOut->findText(exchangeData.account1);
    ui->moneyAccountOut->setCurrentIndex(indexOut);
    int indexIn = ui->moneyAccountIn->findText(exchangeData.account2);
    ui->moneyAccountIn->setCurrentIndex(indexIn);
    ui->moneyTransferSpinBox->setValue(exchangeData.price);

    myExchangeFormTabBase::setUI(exchangeData);
}
void myExchangeFormTransfer::exchangeWindowFeeChanged(double fee) {
    qDebug() << "$$myExchangeFormTransfer::exchangeWindowFeeChanged " << fee << "$$";
    myExchangeFormTabBase::exchangeWindowFeeChanged(fee);
    remainMoneyIn = totalMoneyIn - data.money - fee;
    ui->moneySpinBoxRemainIn->setValue(remainMoneyIn);
}
