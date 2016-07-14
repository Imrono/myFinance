#include "myAssetHistory.h"

#include <QDateTime>
#include <QDebug>

myAssetHistory::myAssetHistory()
    : currentAssetTime(QDateTime::currentDateTime())
{
    historyRoot.initial();
    exchangeListNode.initial();
}

myAssetHistory::~myAssetHistory() {
    exchangeListNode.callback();
    historyRoot.callback();
}

const myAccountAssetRootNode &myAssetHistory::getHistoryNode(const QDateTime &time) {
    // historyRoot calculate
    qint64 refMsec = time.toMSecsSinceEpoch();
    if (refMsec >= currentAssetTime.toMSecsSinceEpoch())
        return historyRoot;

    int exchangeCount = exchangeListNode.getRowCount();
    qint64 lastMsec = exchangeListNode.getDataFromRow(exchangeCount-1).time.toMSecsSinceEpoch();
    for (int i = exchangeCount-1; i >= 0; i--) {
        myExchangeData tmpExchangeData = exchangeListNode.getDataFromRow(i);
        qint64 exchangeMsec = tmpExchangeData.time.toMSecsSinceEpoch();
        if (lastMsec != exchangeMsec)
            lastMsec = exchangeMsec;

        if (exchangeMsec > currentAssetTime.toMSecsSinceEpoch()) {
            continue;
        } else if (exchangeMsec > refMsec) {
            // do sth. with historyRoot using exchangeList.at(i)
            myExchangeData undoExchangeData = -tmpExchangeData;
            doExchangeNode(undoExchangeData);
        }
    }

    currentAssetTime = time;
    return historyRoot;
}

bool myAssetHistory::doExchangeNode(const myExchangeData &exchangeData) {
    bool ans = true;
    const myAccountNode *account = nullptr;
    const myAssetNode   *asset = nullptr;
    myAssetData originalAssetData;

    myAssetData moneyData;
    float money = originalAssetData.price + exchangeData.money;
    moneyData.initMoneyAsset(exchangeData.accountMoney, money);
    account = historyRoot.getAccountNode(exchangeData.accountMoney);
    if (account) {
        asset = myAccountAssetRootNode::getAssetNode(account, MY_CASH);
        if (asset) {    /// update MY_CASH
            originalAssetData = GET_CONST_ASSET_NODE_DATA(asset).assetData;
        } else { }      /// insert MY_CASH

        ans = historyRoot.doChangeAssetNode(moneyData) && ans;
    } else { return false;}

    account = nullptr;
    asset = nullptr;
    originalAssetData.reset();
    myAssetData tmpAssetData = exchangeData.assetData;
    account = historyRoot.getAccountNode(exchangeData.assetData.accountCode);
    if (account) {
        asset = myAccountAssetRootNode::getAssetNode(account, exchangeData.assetData.assetCode);
        if (asset) {    /// update MY_ASSET
            originalAssetData = GET_CONST_ASSET_NODE_DATA(asset).assetData;
        } else { }      /// insert MY_ASSET

        if (exchangeData.assetData.assetCode != MY_CASH) {
            tmpAssetData.amount = originalAssetData.amount + exchangeData.assetData.amount;
            if (0 != tmpAssetData.amount)
                tmpAssetData.price = (-exchangeData.money + originalAssetData.price*originalAssetData.amount)/static_cast<float>(tmpAssetData.amount);
            else {
                qDebug() << "ERROR: 0 == assetData.amount";
                return false;
            }
        } else {
            tmpAssetData.amount = 1;
            tmpAssetData.price = exchangeData.assetData.price + originalAssetData.price;
        }
        ans = historyRoot.doChangeAssetNode(tmpAssetData) && ans;
    } else { return false;}
    return ans;
}
