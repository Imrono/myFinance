#include "myAssetHistory.h"

#include <QDateTime>
#include <QDebug>
#include <QStringList>
QSemaphore myAssetHistory::s(1);
myAssetHistory::myAssetHistory()
    : currentAssetTime(QDateTime::currentDateTime()), assistantThread(this)
{
    // ASSET DATA
    historyRoot.initial();
    qDebug() << historyRoot.toString().toUtf8().data();
    calcCurrentStockHolding();                                  //默认的持有股票信息

    // EXCHANGE DATA
    exchangeListNode.initial();

    // STOCK HISTORY
    stockHistoryData = myStockHistoryData::getInstance();
    s.acquire();
    myAssetHistory::getHistoryDataList(currentStockHolding);    //每只股票的历史价格
    connect(stockHistoryData, SIGNAL(historyDailyDataReady(QString)), this, SLOT(oneStockHistoryDataReady(QString)));
}

myAssetHistory::~myAssetHistory() {
    exchangeListNode.callback();
    historyRoot.callback();
}

//////////////////////
///   NOT TESTED   ///
const myAccountAssetRootNode &myAssetHistory::getHistoryNode(const QDateTime &time) {
    if (leftStock.count()) {
        qDebug() << "ERROR with leftStock.count():" << leftStock.count();
    }
    leftStock.clear();
    // historyRoot calculate
    if (time >= currentAssetTime) {
        return historyRoot;
    } else {
        currentAssetTime = time;
    }

    int exchangeCount = exchangeListNode.getRowCount();
    QDateTime lastTime = exchangeListNode.getDataFromRow(exchangeCount-1).time;
    for (int i = exchangeCount-1; i >= 0; i--) {
        myExchangeData tmpExchangeData = exchangeListNode.getDataFromRow(i);
        if (lastTime != tmpExchangeData.time)
            lastTime = tmpExchangeData.time;

        if (tmpExchangeData.time > currentAssetTime) {
            continue;
        } else if (tmpExchangeData.time > time) {
            // do sth. with historyRoot using exchangeList.at(i)
            myExchangeData undoExchangeData = -tmpExchangeData;
            doExchangeNode(undoExchangeData);
        }
    }

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
        if (MY_CASH == assetData.assetCode) {
            return true;
        } else {
            if (myAccountAssetRootNode::ASSET_INSERT == exchangeType) {
                currentStockHolding.append(assetData.assetCode);
                stockHistoryData->insertStockHistory(assetData.assetCode);
                leftStock.append(assetData.assetCode);
            } else if (myAccountAssetRootNode::ASSET_DELETE == exchangeType) {
                currentStockHolding.removeAll(assetData.assetCode);
                stockHistoryData->deleteStockHistory(assetData.assetCode);
            } else {}
            return true;
        }
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
                        leftStock.append(assetHold.assetCode);
                    }
                }
            }
        }
    }
}
void myAssetHistory::getHistoryDataList(const QList<QString> &currentStockHolding) {
    myStockHistoryData *stockHistoryData = myStockHistoryData::getInstance();
    foreach (QString stockCode, currentStockHolding) {
        stockHistoryData->insertStockHistory(stockCode);
    }
}
void myAssetHistory::calcAssetValueHistory(const QDateTime &from, const QDateTime &to) {
    // ASSET ASSISTANT THREAD
    assistantThread.initialTime(from, to);
    assistantThread.start();
}

void myAssetHistory::oneStockHistoryDataReady(QString stockCode) {
    qDebug() << STR("### oneStockHistoryDataReady:%1 ###").arg(stockCode);
    leftStock.removeAll(stockCode);
    if (0 == leftStock.count()) {
        // CALCULATE ASSET VALUE
        double stockValue = 0.0f;
        // STOCK PART
        foreach (QString tmpStockCode, currentStockHolding) {
            myStockHistoryData::myStockDailyData stockDailyData;
            stockHistoryData->getStockDailyData(tmpStockCode, currentAssetTime, stockDailyData);
            int accountCount = historyRoot.getAccountCount();
            for (int i = 0; i < accountCount; i++) {
                const myAccountNode *account = historyRoot.getAccountNode(i);
                const myAssetNode *asset = myAccountAssetRootNode::getAssetNode(account, tmpStockCode);
                if (asset) {
                    double value = stockDailyData.close*asset->dbAssetData.assetData.amount;
                    stockValue += value;
                    qDebug() << STR("$$ add %1 in %2 with value:%3").arg(asset->dbAssetData.assetData.assetCode)
                                .arg(asset->dbAssetData.assetData.accountCode).arg(value);
                }
            }
        }
        // CASH PART
        int accountCount = historyRoot.getAccountCount();
        for (int i = 0; i < accountCount; i++) {
            const myAccountNode *account = historyRoot.getAccountNode(i);
            if (STR("券商") != account->dbAccountData.accountData.type) {
                continue;
            }
            const myAssetNode *asset = myAccountAssetRootNode::getAssetNode(account, MY_CASH);
            if (asset) {
                stockValue += asset->dbAssetData.assetData.price;
                qDebug() << STR("$$ add %1 in %2 with value:%3").arg(asset->dbAssetData.assetData.assetCode)
                            .arg(asset->dbAssetData.assetData.accountCode).arg(asset->dbAssetData.assetData.price);

            }
        }
        qDebug() << STR("### TOTAL STOCK VALUE %1 in DATE %2 ###").arg(stockValue).arg(currentAssetTime.toString("yyyy-MM-dd"));
        historyValue.insert(currentAssetTime, stockValue);
        s.release();
    }
}


///////////////////////////////////////////////////////////////////////////////////
/// historyValueThread ////////////////////////////////////////////////////////////

historyValueThread::historyValueThread(const QDateTime &from, const QDateTime &to, myAssetHistory *assetHistory)
    : fromTime(from), toTime(to), assetHistory(assetHistory)
{}
historyValueThread::historyValueThread(myAssetHistory *assetHistory)
    : fromTime(QDateTime::currentDateTime()), toTime(QDateTime::currentDateTime()), assetHistory(assetHistory)
{}
historyValueThread::~historyValueThread() {
}

void historyValueThread::run() {
    int i = 0;
    for (QDateTime time = fromTime; time >= toTime; time.addDays(-1)) {
        qDebug() << "### historyValueThread processing " << time.toString("yyyy-MM-dd") <<  "###";
        myAssetHistory::s.acquire();
        myAccountAssetRootNode historyNode = assetHistory->getHistoryNode(time);
        qDebug() << historyNode.toString().toUtf8().data();
        i ++;
        qDebug() << "### historyValueThread processing " << time.toString("yyyy-MM-dd") <<  " finished ###";
    }
    qDebug() << "historyValueThread finished with numOfDays:" << i;
}
