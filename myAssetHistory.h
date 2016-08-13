#ifndef MYASSETHISTORY_H
#define MYASSETHISTORY_H
#include "myGlobal.h"

#include "myAssetNode.h"
#include "myExchangeListNode.h"
#include "myStockHistoryData.h"
#include "myDatabaseDatatype.h"

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
signals:
    void historyValueThreadFinished();
};

#include <QDateTime>
#include <QList>
#include <QMap>
#include <QSemaphore>

class myAssetHistory : public QObject
{
    Q_OBJECT
public:
    friend class historyValueThread;
    myAssetHistory();
    myAssetHistory(const myAccountAssetRootNode &root, const myExchangeListNode &exchangeListNode)
        : currentAssetTime(QDateTime::currentDateTime()), historyRoot(root),
          exchangeListNode(exchangeListNode), assistantThread(this) {}
    ~myAssetHistory();

    const myAccountAssetRootNode *getHistoryNode(const QDateTime &time, QMap<QString, int> &assetChange);

    // TIME POINT SYNC
    static QSemaphore time_s;
    //QSemaphore s_historyTime;
    void prepareCalcAssetValue(const QDateTime &from, const QDateTime &to);
    void doCalcAssetValue(const QDateTime &time);

    void showHistoryValue() {
        QMap<QDateTime, double>::const_iterator ii;
        for (ii = historyValue.constBegin(); ii != historyValue.constEnd(); ++ii) {
            qDebug() << "[" << ii.key().toString("yyyy-MM-dd") << ":" << ii.value() << "]";
        }
    }

    const QMap<QDateTime, double> &getHistoryValue() {
        return historyValue;
    }

private:
    QDateTime currentAssetTime;
    QList<QString> currentStockHolding;
    void calcCurrentStockHolding();
    myStockHistoryData *stockHistoryData;
    QList<QString> leftStock;
    QMap<QDateTime, double> historyValue;
    historyValueThread assistantThread;
    QTime processTime;

    myAccountAssetRootNode historyRoot;
    myExchangeListNode exchangeListNode;

    bool doExchangeNode(const myExchangeData &exchangeData, QMap<QString, int> &assetChange);
    bool doChange(const myAssetData &assetData, QMap<QString, int> &assetChange);

private slots:
    void oneStockHistoryDataReady(QString stockCode);
    void assistantThreadFinished();
signals:
    void ready4Display();

private:
    double calcCurrentAssetValue();
};
#endif // MYASSETHISTORY_H
