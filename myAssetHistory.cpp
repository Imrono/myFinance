#include "myAssetHistory.h"

#include <QDateTime>
#include <QDebug>
#include <QStringList>
QSemaphore myAssetHistory::time_s(1);
myAssetHistory::myAssetHistory()
    : currentAssetTime(QDateTime::currentDateTime()), assistantThread(this)
{
    // ASSET DATA
    historyRoot.initial();
    MY_DEBUG_NODE_ASSET(historyRoot);
    calcCurrentStockHolding();                                  //默认的持有股票信息

    // EXCHANGE DATA
    exchangeListNode.initial();

    // STOCK HISTORY
    stockHistoryData = myStockHistoryData::getInstance();
    if (currentStockHolding.count() > 0) {
        time_s.acquire();
        processTime.start();
        foreach (QString stockCode, currentStockHolding) {
            stockHistoryData->insertStockHistory(stockCode);
        }
    }
    connect(stockHistoryData, SIGNAL(historyDailyDataReady(QString)), this, SLOT(oneStockHistoryDataReady(QString)));
}

myAssetHistory::~myAssetHistory() {
    exchangeListNode.callback();
    historyRoot.callback(STR("myAssetHistory destructor"));
}

//////////////////////
///   NOT TESTED   ///
void myAssetHistory::doCalcAssetValue(const QDateTime &time) {
    QMap<QString, int> assetChange;
    const myAccountAssetRootNode *historyNode = getHistoryNode(time, assetChange);
    Q_UNUSED(historyNode);

    QMapIterator<QString, int> ii(assetChange); 
    if (!ii.hasNext()) {    // 1. 没有asset变化，直接计算value
        double assetValue = calcCurrentAssetValue();
        Q_UNUSED(assetValue);
        time_s.release();
    } else {                // 2. 有asset变化，先取得新asset的历史信息，再计算
         do {
            ii.next();
            calcCurrentStockHolding();
            if (myAccountAssetRootNode::ASSET_INSERT == ii.value()) {
                stockHistoryData->insertStockHistory(ii.key());
            } else if (myAccountAssetRootNode::ASSET_DELETE == ii.value()) {
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

    MY_DEBUG_NODE_ASSET(historyRoot);
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

// currentStockHolding <=> stock in node
void myAssetHistory::calcCurrentStockHolding() {
    QList<QString> prepareRemove = currentStockHolding;
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
                    prepareRemove.removeAll(assetHold.assetCode);
                }
            }
        }
    }

    // REMOVE ASSET INVALID IN NODE
    foreach (QString removeAssetCode, prepareRemove)
        currentStockHolding.removeAll(removeAssetCode);
}

void myAssetHistory::prepareCalcAssetValue(const QDateTime &from, const QDateTime &to) {
    // ASSET ASSISTANT THREAD
    assistantThread.initialTime(from, to);
    if (from < to || from > currentAssetTime) {
        MY_DEBUG_ERROR(STR("SET FROM DATE FAILED!! STOPS"));
        return;
    }

    assistantThread.start();
}

void myAssetHistory::oneStockHistoryDataReady(QString stockCode) {
    leftStock.removeAll(stockCode);
    qDebug() << STR("### oneStockHistoryDataReady: %1, left: %2 ###").arg(stockCode).arg(leftStock.count());
    if (0 == leftStock.count()) {
        double assetValue = calcCurrentAssetValue();
        Q_UNUSED(assetValue);

        time_s.release();
    }
}
double myAssetHistory::calcCurrentAssetValue() {
    // CALCULATE ASSET VALUE
    double assetValue = 0.0f;
    // STOCK PART
    foreach (QString tmpStockCode, currentStockHolding) {
        myStockDailyData stockDailyData;
        stockHistoryData->getStockDailyData(tmpStockCode, currentAssetTime, stockDailyData);
        int accountCount = historyRoot.getAccountCount();
        for (int i = 0; i < accountCount; i++) {
            const myAccountNode *account = historyRoot.getAccountNode(i);
            const myAssetNode *asset = myAccountAssetRootNode::getAssetNode(account, tmpStockCode);
            if (asset) {
                double value = stockDailyData.close*asset->dbAssetData.assetData.amount;
                assetValue += value;

                // TRACE
                QString strTrace;
                strTrace.sprintf("%8.2f(price:%6.2f)", value, stockDailyData.close);
                qDebug() << STR("$$ assetValue + %1 <- [%2 in %3]").arg(strTrace)
                            .arg(asset->dbAssetData.assetData.assetCode)
                            .arg(asset->dbAssetData.assetData.accountCode).toUtf8().data();
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
            assetValue += asset->dbAssetData.assetData.price;

            // TRACE
            QString strTrace;
            strTrace.sprintf("%8.2f              ", asset->dbAssetData.assetData.price);
            qDebug() << STR("$$ assetValue + %1 <- [%2 in %3]").arg(strTrace)
                        .arg(asset->dbAssetData.assetData.assetCode)
                        .arg(asset->dbAssetData.assetData.accountCode).toUtf8().data();
        }
    }
    historyValue.insert(currentAssetTime, assetValue);

    // TRACE
    qDebug() << STR("### TOTAL ASSET VALUE historyValue[%1] = %2 (using %3 Msec) ###")
                .arg(currentAssetTime.toString("yyyy-MM-dd")).arg(assetValue)
                .arg(processTime.elapsed()).toUtf8().data();

    return assetValue;
}

void myAssetHistory::assistantThreadFinished() {
    emit ready4Display();
}

///////////////////////////////////////////////////////////////////////////////////
/// historyValueThread ////////////////////////////////////////////////////////////

historyValueThread::historyValueThread(const QDateTime &from, const QDateTime &to, myAssetHistory *assetHistory)
    : fromTime(from), toTime(to), assetHistory(assetHistory)
{
    connect(this, SIGNAL(historyValueThreadFinished()), assetHistory, SLOT(assistantThreadFinished()));
}
historyValueThread::historyValueThread(myAssetHistory *assetHistory)
    : fromTime(QDateTime::currentDateTime()), toTime(QDateTime::currentDateTime()), assetHistory(assetHistory)
{
    connect(this, SIGNAL(historyValueThreadFinished()), assetHistory, SLOT(assistantThreadFinished()));
}
historyValueThread::~historyValueThread() {
}

void historyValueThread::run() {
    for (QDateTime time = fromTime; time >= toTime; time = time.addDays(-1)) {
        qDebug() << ">>> historyValueThread processing " << time.toString("yyyy-MM-dd") <<  "<<<";
        myAssetHistory::time_s.acquire();
        assetHistory->processTime.start();
        assetHistory->doCalcAssetValue(time);
        qDebug() << "<<< historyValueThread processing " << time.toString("yyyy-MM-dd") <<  " finished >>>";
    }
    qDebug() << "historyValueThread finished with numOfDays:" << toTime.daysTo(fromTime);
    emit historyValueThreadFinished();
}
