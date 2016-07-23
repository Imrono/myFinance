#include "myAssetHistory.h"

#include <QDateTime>
#include <QDebug>
#include <QStringList>
QSemaphore myAssetHistory::s(1);
myAssetHistory::myAssetHistory()
    : currentAssetTime(QDateTime::currentDateTime()), assistantThread(this), lastAssetValue(0.0f)
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
void myAssetHistory::doCalcAssetValue(const QDateTime &time) {
    QMap<QString, int> assetChange;
    const myAccountAssetRootNode *historyNode = getHistoryNode(time, assetChange);
    Q_UNUSED(historyNode);

    QMapIterator<QString, int> ii(assetChange);
    if (!ii.hasNext()) {
        qDebug() << STR("### TOTAL STOCK VALUE %1 in DATE %2 (not changed) ###").arg(lastAssetValue).arg(currentAssetTime.toString("yyyy-MM-dd"));
        historyValue.insert(currentAssetTime, lastAssetValue);
        s.release();
    } else {
         do {
            ii.next();
            if (myAccountAssetRootNode::ASSET_INSERT == ii.value()) {
                currentStockHolding.append(ii.key());
                stockHistoryData->insertStockHistory(ii.key());
                leftStock.append(ii.key());
            } else if (myAccountAssetRootNode::ASSET_DELETE == ii.value()) {
                currentStockHolding.removeAll(ii.key());
                stockHistoryData->deleteStockHistory(ii.key());
            } else if (myAccountAssetRootNode::ASSET_MODIFY == ii.value()) {
                continue;
            } else {}
        } while (ii.hasNext());
    }
}
const myAccountAssetRootNode *myAssetHistory::getHistoryNode(const QDateTime &time, QMap<QString, int> &assetChange) {
    if (leftStock.count()) {
        MY_DEBUG_ERROR(STR("ERROR with leftStock.count():%1").arg(leftStock.count()));
    }
    leftStock.clear();
    // historyRoot CALCUlATE
    // 1. 暂不支持时间回退，time > currentAssetTime
    if (time > currentAssetTime) {
        MY_DEBUG_ERROR(STR("DATETIME:%1 > currentAssetTime:%2, CAN NOT PROCEDSS").arg(time.toString("yyyy-MM-dd")).arg(currentAssetTime.toString("yyyy-MM-dd")));
        return nullptr;
    } else if (time == currentAssetTime) {
        qDebug() << "DATETIME == currentAssetTime -> " << time.toString("yyyy-MM-dd");
        return &historyRoot;
    }

    int exchangeCount = exchangeListNode.getRowCount();
    QDateTime fromDuring = currentAssetTime, toDuring = currentAssetTime;
    for (int i = exchangeCount-1; i >= 0; i--) {
        myExchangeData tmpExchangeData = exchangeListNode.getDataFromRow(i);

        // x日的value计算 -> x+1日的exchangeData
        if (tmpExchangeData.time > currentAssetTime) {
            continue;
        } else if (tmpExchangeData.time > time) {
            // do sth. with historyRoot using exchangeList.at(i)
            myExchangeData undoExchangeData = -tmpExchangeData;
            doExchangeNode(undoExchangeData, assetChange);
            currentAssetTime = tmpExchangeData.time.addDays(-1);
            fromDuring = currentAssetTime;
        } else {
            break;
        }
    }
    currentAssetTime = time;
    toDuring  = currentAssetTime;

    qDebug() << historyRoot.toString().toUtf8().data();
    return &historyRoot;
}
bool myAssetHistory::doExchangeNode(const myExchangeData &exchangeData, QMap<QString, int> &assetChange) {
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

        ans = doChange(moneyData, assetChange) && ans;
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
        ans = doChange(tmpAssetData, assetChange) && ans;
    } else { return false;}
    return ans;
}
bool myAssetHistory::doChange(const myAssetData &assetData, QMap<QString, int> &assetChange) {
    int exchangeType = -1;
    if (historyRoot.doChangeAssetNode(assetData, exchangeType)) {
        if (MY_CASH == assetData.assetCode) {
            return true;
        } else {
            assetChange[assetData.assetCode] = exchangeType;
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
void myAssetHistory::prepareCalcAssetValue(const QDateTime &from, const QDateTime &to) {
    // ASSET ASSISTANT THREAD
    assistantThread.initialTime(from, to);
    QMap<QString, int> assetChange;
    if (!getHistoryNode(from, assetChange)) {
        MY_DEBUG_ERROR("SET FROM DATE FAILED!! STOPS");
        return;
    }

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
        lastAssetValue = stockValue;
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
    for (QDateTime time = fromTime; time >= toTime; time = time.addDays(-1)) {
        qDebug() << "### historyValueThread processing " << time.toString("yyyy-MM-dd") <<  "###";
        myAssetHistory::s.acquire();
        assetHistory->doCalcAssetValue(time);
        qDebug() << "### historyValueThread processing " << time.toString("yyyy-MM-dd") <<  " finished ###";
    }
    qDebug() << "historyValueThread finished with numOfDays:" << toTime.daysTo(fromTime);
}
