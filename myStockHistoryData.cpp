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

void myStockHistoryData::getStockHistory(QString stockCode) {
    QString preStr = stockCode.left(3);
    if ("sh." == preStr) {
        stockCode.remove(0, 3);
        stockCode.append(STR(".ss"));
    } else if ("sz." == preStr) {
        stockCode.remove(0, 3);
        stockCode.append(STR(".sz"));
    } else {}
    lastStockCode = stockCode;
    if (stockHistoryList.contains(stockCode)) {
        if (pendingRemoveStock.contains(stockCode)) {
            pendingRemoveStock.removeAll(stockCode);
        } else {}
        qDebug() << STR("### %1 already exist in list ###").arg(stockCode);
        return;
    } else {
        stockHistoryList.insert(stockCode, nullptr);
    }

    historyDailyDataProcessThread *thread = new historyDailyDataProcessThread(stockCode, this);
    threads.insert(stockCode, thread);
    connect(thread, SIGNAL(processFinish(QString)), this, SLOT(oneHistoryDailyDataInserted(QString)));
}

void myStockHistoryData::oneHistoryDailyDataInserted(const QString stockCode) {
    qDebug() << STR("%1 historyDailyData process finished").arg(stockCode);
    historyDailyDataProcessThread *thread = threads.value(stockCode);
    disconnect(thread, SIGNAL(processFinish(QString)), this, SLOT(oneHistoryDailyDataInserted(QString)));
    delete thread;
    threads.remove(stockCode);

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
    qDebug() << "myStockHistoryData::replyFinished with stockHistoryList.count():" << stockHistoryList.count()
             << " pendingRemoveStock.count()" << pendingRemoveStock.count();
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
