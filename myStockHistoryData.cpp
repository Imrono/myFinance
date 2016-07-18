#include "myStockHistoryData.h"
#include "myDatabaseDatatype.h"

#include <QByteArray>
#include <QDebug>
#include <QMessageBox>
#include <QEventLoop>

historyDailyDataProcessThread::historyDailyDataProcessThread(const QString &stockCode, myStockHistoryData* parent)
    : parent(parent), stockCode(stockCode), QThread(parent) {}

historyDailyDataProcessThread::~historyDailyDataProcessThread() {}

void historyDailyDataProcessThread::run() {
    qDebug() << STR("### historyDailyDataProcessThread |%1| run ###").arg(stockCode);

    QString urlYahooHistory;
    QString prefix = STR("http://table.finance.yahoo.com/table.csv?s=");
    urlYahooHistory = prefix + stockCode;

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkReply *reply = manager->get(QNetworkRequest(QUrl(urlYahooHistory)));
    MY_DEBUG_URL(urlYahooHistory);
    QEventLoop eventLoop;
    connect(manager, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
    eventLoop.exec();       //block until finish

    QByteArray lineData = reply->readLine();
    QList<myStockHistoryData::myStockDailyData> *tmpStockHistoryList = new QList<myStockHistoryData::myStockDailyData>();
    unsigned historyCount = 0;
    while (!lineData.isNull()) {
        //qDebug() << stockDailyData;
        QList<QByteArray> strList = lineData.split(',');

        if (7 == strList.count()) {
            myStockHistoryData::myStockDailyData stockDailyData;
            stockDailyData.datetime = QDateTime::fromString(strList.at(0), "yyyy-MM-dd");
            stockDailyData.open     = strList.at(1).toFloat();
            stockDailyData.high     = strList.at(2).toFloat();
            stockDailyData.low      = strList.at(3).toFloat();
            stockDailyData.close    = strList.at(4).toFloat();
            stockDailyData.volume   = strList.at(5).toInt();
            stockDailyData.adjClose = strList.at(6).toFloat();

            tmpStockHistoryList->append(stockDailyData);
            historyCount ++;
            lineData = reply->readLine();
        }
    }
    qDebug() << "myStockHistoryData::replyFinished with historyCount:" << historyCount;

    mutex.lock();
    parent->stockHistoryList[stockCode] = tmpStockHistoryList;
    mutex.unlock();
    emit processFinish(stockCode);
}

/////////////////////////////////////
/// class myStockHistoryData PART ///
/////////////////////////////////////
myStockHistoryData *myStockHistoryData::instance = nullptr;
myStockHistoryData::myStockHistoryData() : maxNumOfHistories(10)
{}
myStockHistoryData::~myStockHistoryData() {
    while (stockHistoryList.count()) {
        if (stockHistoryList.first()) {
            delete stockHistoryList.first();
            stockHistoryList.remove(stockHistoryList.firstKey());
        }
    }
}

void myStockHistoryData::insertStockHistory(const QString &stockCode) {
    lastStockCode = stockCode;
    if (stockHistoryList.contains(stockCode)) {
        if (pendingRemoveStock.contains(stockCode)) {
            pendingRemoveStock.removeAll(stockCode);
        } else {}
        qDebug() << STR("### historyDailyDataReady -> %1 already exist in list ###").arg(stockCode);
        emit historyDailyDataReady(stockCode);
        return;
    } else {
        removePendingDelete();

        stockHistoryList.insert(stockCode, nullptr);    //nullptr在线程中被更新
        historyDailyDataProcessThread *thread = new historyDailyDataProcessThread(stockCode2YahooStyle(stockCode), this);
        threads.insert(stockCode, thread);
        connect(thread, SIGNAL(processFinish(QString)), this, SLOT(oneHistoryDailyDataInserted(QString)));
        thread->start();
        return;
    }
}
void myStockHistoryData::deleteStockHistory(const QString &stockCode) {
    pendingRemoveStock.append(stockCode);
    removePendingDelete();
}

void myStockHistoryData::oneHistoryDailyDataInserted(const QString stockCode) {
    qDebug() << STR("%1 historyDailyData process finished").arg(stockCode);
    historyDailyDataProcessThread *thread = threads.value(stockCode);
    disconnect(thread, SIGNAL(processFinish(QString)), this, SLOT(oneHistoryDailyDataInserted(QString)));
    delete thread;
    threads.remove(stockCode);

    emit historyDailyDataReady(stockCode);
    qDebug() << "### historyDailyDataReady -> stockHistoryList.count():" << stockHistoryList.count()
             << STR(" pendingRemoveStock.count():%1 ###").arg(pendingRemoveStock.count());
}

void myStockHistoryData::removePendingDelete() {
    if (stockHistoryList.count() > maxNumOfHistories) {
        int pendingRemoveCount = pendingRemoveStock.count();
        if (pendingRemoveCount > 0) {
            for (int i = 0; i < pendingRemoveCount; i++) {
                stockHistoryList.remove(pendingRemoveStock.first());
                pendingRemoveStock.removeFirst();
                if (stockHistoryList.count() <= maxNumOfHistories) {
                    break;
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool myStockHistoryData::getStockDailyData(const QString &stockCode, const QDateTime dateTime, myStockDailyData &stockDailyData) {
    if (!stockHistoryList.contains(stockCode))
        return false;

    QList<myStockDailyData> *historyData = stockHistoryList.value(stockCode);
    int historyDailyDataCount = historyData->count();
    for (int i = 0; i < historyDailyDataCount; i++) {
        const myStockDailyData &dailyData = historyData->at(i);
        if (dailyData.datetime == dateTime) {
            stockDailyData = dailyData;
            return true;
        } else if (dailyData.datetime > dateTime) {
            continue;
        } else {
            return false;
        }
    }
    return false;
}

QString myStockHistoryData::stockCode2YahooStyle(const QString &stockCode) {
    QString tmpStockCode = stockCode;
    QString preStr = tmpStockCode.left(3);
    if ("sh." == preStr) {
        tmpStockCode.remove(0, 3);
        tmpStockCode.append(STR(".ss"));
    } else if ("sz." == preStr) {
        tmpStockCode.remove(0, 3);
        tmpStockCode.append(STR(".sz"));
    } else {}
    return tmpStockCode;
}
