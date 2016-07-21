#include "myExchangeFormTabBase.h"
#include "myFinanceExchangeWindow.h"

myExchangeFormTabBase::myExchangeFormTabBase(const myAccountAssetRootNode *rootNode, QString tabName, int tabType, QWidget *parent, bool isModifyExchange)
    : parent(static_cast<myFinanceExchangeWindow *>(parent)), QWidget(parent), tabType(tabType),
      isModifyExchange(isModifyExchange), totalMoney(0.0f), remainMoney(0.0f),
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
    data.id = exchangeData.id;
    if (qAbs(exchangeData.money) > MONEY_EPS) {
        usedMoneyBeforeModify = exchangeData.money;
        usedFeeBeforeModify = exchangeData.assetData.price*static_cast<float>(exchangeData.assetData.amount) - exchangeData.money;
    }
}
void myExchangeFormTabBase::checkAndSetDisable(const myExchangeData &exchangeData) {
    Q_UNUSED(exchangeData);
}
void myExchangeFormTabBase::disableAll() {

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
