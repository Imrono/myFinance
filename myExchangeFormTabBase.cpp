#include "myExchangeFormTabBase.h"
#include "myFinanceExchangeWindow.h"

myExchangeFormTabBase::myExchangeFormTabBase(const myAccountAssetRootNode *rootNode, QString tabName, int tabType, QWidget *parent, bool isModifyExchange)
    : parent(static_cast<myFinanceExchangeWindow *>(parent)), QWidget(parent), tabType(tabType),
      isModifyExchange(isModifyExchange),
      usedMoneyBeforeModify(0.0f), usedFeeBeforeModify(0.0f),
      rootNode(rootNode), tabName(tabName)
{
    data.exchangeType = this->tabName;
    qDebug() << "tab " << tabName << " is Ininialized with tabName " << this->tabName;
}

void myExchangeFormTabBase::recordExchangeData(myExchangeData &tmpData) {
    parent->getCommonExchangeData(tmpData);
}
void myExchangeFormTabBase::setUI(const myExchangeData &exchangeData) {
    if (qAbs(exchangeData.money) > MONEY_EPS) {
        usedMoneyBeforeModify = exchangeData.money;
        usedFeeBeforeModify = exchangeData.assetData.price*static_cast<float>(exchangeData.assetData.amount) - exchangeData.money;
    }
}
void myExchangeFormTabBase::checkAndSetDisable(const myExchangeData &exchangeData) {
    Q_UNUSED(exchangeData);
}

void myExchangeFormTabBase::setExchangeWindowType(const QString &type) {
    parent->setExchangeWindowUiType(type);
}
void myExchangeFormTabBase::setExchangeWindowFee(double fee) {
    parent->setExchangeWindowUiFee(fee);
}
void myExchangeFormTabBase::recoverTypeAndFee() {
    setExchangeWindowType(data.exchangeType);
    setExchangeWindowFee(data.fee);
}

void myExchangeFormTabBase::exchangeWindowFeeChanged(double fee) {
    data.fee = fee;
}

float myExchangeFormTabBase::getTotalMoney(int index) {
    float tmpTotalMoney = 0.0f;
    myAccountNode *accountNode = rootNode->getAccountNode(index);
    int numAsset = accountNode->children.size();
    for (int i = 0; i < numAsset; i++) {
        const myAssetNode *asset = static_cast<const myAssetNode *>(accountNode->children.at(i));
        const myAssetNodeData &assetHold = GET_CONST_ASSET_NODE_DATA(asset);
        if (assetHold.assetData.assetCode == MY_CASH) {
            qDebug() << assetHold.assetData.accountCode << " " << assetHold.assetData.assetCode << " " << assetHold.assetData.price;
            tmpTotalMoney = assetHold.assetData.price;
            break;
        }
    }
    return tmpTotalMoney;
}

const myExchangeData &myExchangeFormTabBase::getExchangeData() {
    return data;
}

void myExchangeFormTabBase::traceExchangeData() {
    qDebug() << "##CURRENT TAB: " << tabName << "##";
    qDebug() << "data.time "     << data.time << ","
             << "data.type "     << data.exchangeType << ","
             << "data.accountMoney " << data.accountMoney << ","
             << "data.money "    << data.money << "\n"
             << "data.account2 " << data.assetData.accountCode << ","
             << "data.code "     << data.assetData.assetCode << ","
             << "data.name "     << data.assetData.assetName << ","
             << "data.amount "   << data.assetData.amount << ","
             << "data.price "    << data.assetData.price;
}
