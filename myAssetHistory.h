#ifndef MYASSETHISTORY_H
#define MYASSETHISTORY_H
#include "myGlobal.h"

#include "myAssetNode.h"
#include "myExchangeListNode.h"
#include "myStockHistoryData.h"

///////////////////////////////////////////////////////////////////////////////////
/// historyValueThread ////////////////////////////////////////////////////////////
#include <QThread>
#include <QSemaphore>
#include <QDateTime>
class myAssetHistory;
class historyValueThread : public QThread
{
    Q_OBJECT
public:
    historyValueThread(const QDateTime &from, const QDateTime &to, myAssetHistory *assetHistory);
    historyValueThread(myAssetHistory *assetHistory);
    ~historyValueThread();

    void initialTime(const QDateTime &from, const QDateTime &to) {
        fromTime = from;
        toTime   = to;
    }

protected:
    void run() Q_DECL_OVERRIDE;
private:
    QDateTime fromTime;
    QDateTime toTime;
    myAssetHistory *assetHistory;
};

#include <QDateTime>
#include <QList>
#include <QMap>
#include <QSemaphore>

class myAssetHistory : public QObject
{
    Q_OBJECT
public:
    myAssetHistory();
    myAssetHistory(const myAccountAssetRootNode &root, const myExchangeListNode &exchangeListNode)
        : currentAssetTime(QDateTime::currentDateTime()), historyRoot(root),
          exchangeListNode(exchangeListNode), assistantThread(this) {}
    ~myAssetHistory();

    const myAccountAssetRootNode &getHistoryNode(const QDateTime &time);
    bool doExchangeNode(const myExchangeData &exchangeData);

    static void getHistoryDataList(const QList<QString> &currentStockHolding);
    static QSemaphore s;
    void calcAssetValueHistory(const QDateTime &from, const QDateTime &to);

private:
    QDateTime currentAssetTime;
    QList<QString> currentStockHolding;
    void calcCurrentStockHolding();
    myStockHistoryData *stockHistoryData;
    QList<QString> leftStock;
    QMap<QDateTime, double> historyValue;
    historyValueThread assistantThread;

    myAccountAssetRootNode historyRoot;
    myExchangeListNode exchangeListNode;

    bool doChange(const myAssetData &assetData);

private slots:
    void oneStockHistoryDataReady(QString stockCode);
};
#endif // MYASSETHISTORY_H
