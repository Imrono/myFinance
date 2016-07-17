#include "myAssetHistory.h"

#include <QDateTime>
#include <QDebug>

myAssetHistory::myAssetHistory()
    : currentAssetTime(QDateTime::currentDateTime())
{
    // ASSET DATA
    historyRoot.initial();
    calcCurrentStockHolding();                                  //默认的持有股票信息
    stockHistoryData = myStockHistoryData::getInstance();
    myAssetHistory::getHistoryDataList(currentStockHolding);    //每只股票的历史价格

    // EXCHANGE DATA
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

        ans = doChange(moneyData) && ans;
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
                tmpAssetData.price = (-exchangeData.money + originalAssetData.price*originalAssetData.amount)/tmpAssetData.amount;
            else {
                qDebug() << "ERROR: 0 == assetData.amount";
                return false;
            }
        } else {
            tmpAssetData.amount = 1;
            tmpAssetData.price = exchangeData.assetData.price + originalAssetData.price;
        }
        ans = doChange(tmpAssetData) && ans;
    } else { return false;}
    return ans;
}
bool myAssetHistory::doChange(const myAssetData &assetData) {
    int exchangeType = -1;
    if (historyRoot.doChangeAssetNode(assetData, exchangeType)) {
        if (myAccountAssetRootNode::ASSET_INSERT == exchangeType) {
            currentStockHolding.append(assetData.assetCode);
            myAssetHistory::getHistoryDataList(currentStockHolding);
        } else if (myAccountAssetRootNode::ASSET_DELETE == exchangeType) {
            currentStockHolding.removeAll(assetData.assetCode);
            myAssetHistory::getHistoryDataList(currentStockHolding);
        } else {}
        return true;
    } else {
        return false;
    }
}

void myAssetHistory::calcCurrentStockHolding() {
    currentStockHolding.clear();
    const myRootNode *rootNode = historyRoot.getRootNode();
    int accountCount = historyRoot.getAccountCount();
    for (int i = 0; i < accountCount; i++) {
        const myAccountNode *account = static_cast<const myAccountNode *>(rootNode->children.at(i));
        if (STR("券商") != account->dbAccountData.accountData.type) {
            continue;
        } else {
            int assetCount = account->children.count();
            for (int j = 0; j < assetCount; j++) {
                const myAssetNode *asset = static_cast<const myAssetNode *>(account->children.at(j));
                if (STR("股票") != asset->dbAssetData.assetData.type) {
                    continue;
                } else {
                    const myAssetData &assetHold = asset->dbAssetData.assetData;
                    if (!currentStockHolding.contains(assetHold.assetCode)) {
                        currentStockHolding.append(assetHold.assetCode);
                    }
                }
            }
        }
    }
}

void myAssetHistory::getHistoryDataList(const QList<QString> &currentStockHolding) {
    myStockHistoryData *stockHistoryData = myStockHistoryData::getInstance();
    foreach (QString stockCode, currentStockHolding) {
        stockHistoryData->getStockHistory(stockCode);
    }
}
