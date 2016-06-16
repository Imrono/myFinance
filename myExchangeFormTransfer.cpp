#include "myExchangeFormTransfer.h"
#include "ui_myExchangeFormTransfer.h"
#include "AssetCode2Type.h"

myExchangeFormTransfer::myExchangeFormTransfer(const myRootAccountAsset *rootNode, QString tabName, QWidget *parent) :
    myExchangeFormTabBase(rootNode, tabName, myExchangeUI::TAB_TRANS, parent),
    accountNodeOut(nullptr), accountNodeIn(nullptr),
    ui(new Ui::myExchangeFormTransfer)
{
    ui->setupUi(this);

    data.assetData.assetCode = MY_CASH;
    data.assetData.amount    = 1;
    data.assetData.type = AssetCode2Type::getInstance()->getAssetType(data.assetData.assetCode);

    inIdx2AccountIdx.clear();
    outIdx2AccountIdx.clear();
    int localCount = 0;
    for (int i = 0; i < rootNode->getAccountCount(); i++) {
        myAssetNode *accountNode = rootNode->getAccountNode(i);
        for (int j = 0; j < accountNode->children.count(); j++) {
            myAssetNode *holdNode = accountNode->children.at(j);
            QString assetCode = holdNode->nodeData.value<myAssetHold>().assetData.assetCode;
            if (assetCode.contains("cash")) {
                const myAssetAccount &accountData = accountNode->nodeData.value<myAssetAccount>();
                QIcon   icon =QIcon( QString(":/icon/finance/resource/icon/finance/%1").arg(accountData.logo));
                QString code;
                if (accountData.accountData.name.contains(STR("ÒøÐÐ"))) {
                    code = "**** **** " + accountData.accountData.code.right(4);
                } else {
                    code = accountData.accountData.code;
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
    // 1. data.accountMoney & data.account2 update
    accountNodeOut = rootNode->getAccountNode(nodeIdx);
    const myAssetAccount accountData = accountNodeOut->nodeData.value<myAssetAccount>();
    data.accountMoney = accountData.accountData.code;

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
    // 1. accountIn update
    accountNodeIn = rootNode->getAccountNode(nodeIdx);
    const myAssetAccount accountData = accountNodeIn->nodeData.value<myAssetAccount>();
    data.assetData.accountCode = accountData.accountData.code;
    for (int j = 0; j < accountNodeIn->children.count(); j++) {
        myAssetNode *holdNode = accountNodeIn->children.at(j);
        QString assetCode = holdNode->nodeData.value<myAssetHold>().assetData.assetCode;
        if (assetCode.contains("cash")) {
            data.assetData.assetName = holdNode->nodeData.value<myAssetHold>().assetData.assetName;
            break;
        }
    }
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
    data.assetData.price = value;
    data.money = -data.assetData.price - data.fee;

    remainMoneyOut = totalMoneyOut + data.money;
    ui->moneySpinBoxRemainOut->setValue(remainMoneyOut);
    remainMoneyIn = totalMoneyIn + data.assetData.price;
    ui->moneySpinBoxRemainIn->setValue(remainMoneyIn);
}

void myExchangeFormTransfer::recordExchangeData(myExchangeData &tmpData) {
    myExchangeFormTabBase::recordExchangeData(tmpData);

    if (accountNodeOut)
        tmpData.accountMoney = accountNodeOut->nodeData.value<myAssetAccount>().accountData.code;
    tmpData.money    = -ui->moneyTransferSpinBox->value() - data.fee;

    if (accountNodeIn)
        tmpData.assetData.accountCode = accountNodeIn->nodeData.value<myAssetAccount>().accountData.code;
    tmpData.assetData.assetCode   = MY_CASH;
    tmpData.assetData.assetName   = data.assetData.assetName;
    tmpData.assetData.amount      = 1;
    tmpData.assetData.price       = ui->moneyTransferSpinBox->value();
    tmpData.assetData.type        = data.assetData.type;
}
void myExchangeFormTransfer::setUI(const myExchangeData &exchangeData) {
    myAssetNode *accountNode1 = rootNode->getAccountNode(exchangeData.accountMoney);
    myAssetNode *accountNode2 = rootNode->getAccountNode(exchangeData.assetData.accountCode);
    for (int i = 0; i < rootNode->getAccountCount(); i++) {
        myAssetNode *accountNode = rootNode->getAccountNode(i);
        if (accountNode1 == accountNode) {
            int localIndex = outIdx2AccountIdx.find(i).value();
            ui->moneyAccountOut->setCurrentIndex(localIndex);
        }
        if (accountNode2 == accountNode) {
            int localIndex = inIdx2AccountIdx.find(i).value();
            ui->moneyAccountIn->setCurrentIndex(localIndex);
        }
    }
    ui->moneyTransferSpinBox->setValue(exchangeData.assetData.price);

    myExchangeFormTabBase::setUI(exchangeData);
}
void myExchangeFormTransfer::checkAndSetDisable(const myExchangeData &exchangeData) {
    setUI(exchangeData);
    if (exchangeData.accountMoney != "")
        ui->moneyAccountOut->setDisabled(true);
    if (exchangeData.assetData.accountCode != "")
        ui->moneyAccountIn->setDisabled(true);
}
void myExchangeFormTransfer::exchangeWindowFeeChanged(double fee) {
    qDebug() << "$$myExchangeFormTransfer::exchangeWindowFeeChanged " << fee << "$$";
    myExchangeFormTabBase::exchangeWindowFeeChanged(fee);
    data.money = -data.assetData.price - data.fee;
    remainMoneyOut = totalMoneyOut + data.money;
    ui->moneySpinBoxRemainOut->setValue(remainMoneyOut);
}
