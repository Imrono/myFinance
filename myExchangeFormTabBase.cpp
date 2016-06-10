#include "myExchangeFormTabBase.h"
#include "myFinanceExchangeWindow.h"

myExchangeFormTabBase::myExchangeFormTabBase(const myRootAccountAsset *rootNode, QString tabName, int tabType, QWidget *parent)
    : parent(static_cast<myFinanceExchangeWindow *>(parent)), QWidget(parent), tabType(tabType),
      isModifyExchange(false),
      usedMoneyBeforeModify(0.0f), usedFeeBeforeModify(0.0f),
      rootNode(rootNode), tabName(tabName)
{
    data.type = this->tabName;
    //myExchangeFormTabBase::recordExchangeData(data);
    qDebug() << "tab " << tabName << " is Ininialized with tabName " << this->tabName;
}

void myExchangeFormTabBase::recordExchangeData(myExchangeData &tmpData) {
    parent->getCommonExchangeData(tmpData);
}
void myExchangeFormTabBase::setUI(const myExchangeData &exchangeData) {
    if (qAbs(exchangeData.money) > MONEY_EPS) {
        usedMoneyBeforeModify = exchangeData.money;
        usedFeeBeforeModify = exchangeData.price*static_cast<float>(exchangeData.amount) - exchangeData.money;
    }
}
void myExchangeFormTabBase::checkAndSetDisable(const myExchangeData &exchangeData) {

}

void myExchangeFormTabBase::setExchangeWindowType(const QString &type) {
    parent->setExchangeWindowUiType(type);
}
void myExchangeFormTabBase::setExchangeWindowFee(double fee) {
    parent->setExchangeWindowUiFee(fee);
}
void myExchangeFormTabBase::recoverTypeAndFee() {
    setExchangeWindowType(data.type);
    setExchangeWindowFee(data.fee);
}

void myExchangeFormTabBase::exchangeWindowFeeChanged(double fee) {
    data.fee = fee;
}

float myExchangeFormTabBase::getTotalMoney(int index) {
    float tmpTotalMoney = 0.0f;
    myAssetNode *accountNode = rootNode->getAccountNode(index);
    int numAsset = accountNode->children.size();
    for (int i = 0; i < numAsset; i++) {
        myAssetNode *asset = accountNode->children.at(i);
        const myAssetHold &assetHold = (asset->nodeData).value<myAssetHold>();
        if (assetHold.assetCode == MY_CASH) {
            qDebug() << assetHold.accountCode << " " << assetHold.assetCode << " " << assetHold.price;
            tmpTotalMoney = assetHold.price;
            break;
        }
    }
    return tmpTotalMoney;
}

void myExchangeFormTabBase::traceExchangeData() {
    qDebug() << "##CURRENT TAB: " << tabName << "##";
    qDebug() << "data.time "     << data.time << ","
             << "data.type "     << data.type << ","
             << "data.account1 " << data.account1 << ","
             << "data.money "    << data.money << "\n"
             << "data.account2 " << data.account2 << ","
             << "data.code "     << data.code << ","
             << "data.name "     << data.name << ","
             << "data.amount "   << data.amount << ","
             << "data.price "    << data.price;
}
