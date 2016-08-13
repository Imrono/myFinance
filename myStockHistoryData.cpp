#include "myStockHistoryData.h"
#include "myDatabaseDatatype.h"

#include <QByteArray>
#include <QDebug>
#include <QMessageBox>
#include <QEventLoop>

historyDailyDataProcessThread::historyDailyDataProcessThread(const QString &stockCode, myStockHistoryData* parent)
    : parent(parent), stockCode(stockCode) {}

historyDailyDataProcessThread::~historyDailyDataProcessThread() {}

void historyDailyDataProcessThread::run() {
    qDebug() << STR("### historyDailyDataProcessThread |%1| run ###").arg(stockCode);

    //QString url = stockUrlYahoo(stockCode);
    QString url = stockUrlNetEase(stockCode);

    QNetworkAccessManager *manager = new QNetworkAccessManager();
    reply = manager->get(QNetworkRequest(QUrl(url)));
    MY_DEBUG_URL(url);
    connect(manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(stockHistoryFinished(QNetworkReply *)));
    //connect(reply, SIGNAL(readyRead()), this, SLOT(stockReadyRead()));
    //connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(stockDownloadProgress(qint64, qint64)));

    // THREAD EVENT LOOP
    exec();
    if (manager) {
        delete manager;
        manager = nullptr;
    }
    qDebug() << "### ThreadId:" << QThread::currentThreadId() << STR("(%1 historyCount:%2) THREAD eventLoop finished!!! ###")
                .arg(stockCode).arg(parent->stockHistoryList[stockCode]->count()).toUtf8().data();
    emit processFinish(stockCode);
}
void historyDailyDataProcessThread::stockHistoryFinished(QNetworkReply *reply) {
    QByteArray lineData = reply->readLine();
    QList<myStockDailyData> *tmpStockHistoryList = new QList<myStockDailyData>();
    unsigned historyCount = 0;
    while (!lineData.isNull()) {
        if ("sz.000651" == stockCode && historyCount < 10) {
            qDebug() << stockCode << ": " << QString(lineData);
        }

        myStockDailyData stockDailyData;
        //if (stockStoreYahoo(lineData, stockDailyData)) {
        if (stockStoreNetEase(lineData, stockDailyData)) {
            tmpStockHistoryList->append(stockDailyData);
            historyCount ++;
        }
        lineData = reply->readLine();
    }
    parent->stockHistoryList[stockCode] = tmpStockHistoryList;
    emit quit();
}
void historyDailyDataProcessThread::stockReadyRead() {
    qDebug() << stockCode << " readBufferSize:" << reply->readBufferSize();
}
void historyDailyDataProcessThread::stockDownloadProgress(qint64 bytesReceived, qint64 bytesTotal) {
    float percentage = 0.0f;
    if (0 != bytesTotal && -1 != bytesTotal) {
        percentage = static_cast<float>(bytesReceived)/static_cast<float>(bytesTotal);
    } else {
        bytesTotal = 0;
    }
    qDebug() << stockCode << " received:" << static_cast<float>(bytesReceived)/1024 << "K,"
             << "total:" << static_cast<float>(bytesTotal)/1024 << "K,"
             << "percentage: " << percentage*100 << "%";
}

QString historyDailyDataProcessThread::stockUrlYahoo(const QString &stockCode) {
    return STR("http://table.finance.yahoo.com/table.csv?s=%1").arg(stockCode2YahooStyle(stockCode));
}
QString historyDailyDataProcessThread::stockCode2YahooStyle(const QString &stockCode) {
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
bool historyDailyDataProcessThread::stockStoreYahoo(QByteArray &lineData, myStockDailyData &stockDailyData) {
    if (lineData == "Date,Open,High,Low,Close,Volume,Adj Close\n") {
        return false;
    }
    //qDebug() << stockDailyData;
    lineData.remove(lineData.lastIndexOf('\n'), 1);
    QList<QByteArray> strList = lineData.split(',');

    if (7 == strList.count()) {
        stockDailyData.datetime = QDateTime::fromString(strList.at(0), "yyyy-MM-dd");
        stockDailyData.open     = strList.at(1).toFloat();
        stockDailyData.high     = strList.at(2).toFloat();
        stockDailyData.low      = strList.at(3).toFloat();
        stockDailyData.close    = strList.at(4).toFloat();
        stockDailyData.volume   = strList.at(5).toInt();
        stockDailyData.adjClose = strList.at(6).toFloat();

        return true;
    }
    return false;
}

QString historyDailyDataProcessThread::stockCode2NetEaseStyle(const QString &stockCode) {
    QString tmpStockCode = stockCode;
    QString preStr = tmpStockCode.left(3);
    if ("sh." == preStr) {
        tmpStockCode.remove(0, 3);
        tmpStockCode.insert(0, "0");
    } else if ("sz." == preStr) {
        tmpStockCode.remove(0, 3);
        tmpStockCode.insert(0, "1");
    } else {}
    return tmpStockCode;
}
QString historyDailyDataProcessThread::stockUrlNetEase(const QString &stockCode) {
    return STR("http://quotes.money.163.com/service/chddata.html?"
               "code=%1&start=00000000&end=%2"
               "&fields=TCLOSE;HIGH;LOW;TOPEN;LCLOSE;CHG;PCHG;TURNOVER;VOTURNOVER;VATURNOVER;TCAP;MCAP")
            .arg(stockCode2NetEaseStyle(stockCode))
            .arg(QDate::currentDate().toString("yyyyMMdd"));
}
bool historyDailyDataProcessThread::stockStoreNetEase(QByteArray &lineData, myStockDailyData &stockDailyData) {
    QString title = STR("日期,股票代码,名称,收盘价,最高价,最低价,开盘价,前收盘,涨跌额,涨跌幅,换手率,成交量,成交金额,总市值,流通市值");
    QString strData = STR(lineData);

    if (strData.contains(title)) {
        return false;
    }
    lineData.remove(lineData.lastIndexOf('\n'), 1);
    QList<QByteArray> strList = lineData.split(',');

    if (15 == strList.count()) {
        stockDailyData.datetime = QDateTime::fromString(strList.at(0), "yyyy-MM-dd");
        stockDailyData.open     = strList.at(6).toFloat();
        stockDailyData.high     = strList.at(4).toFloat();
        stockDailyData.low      = strList.at(5).toFloat();
        stockDailyData.volume   = strList.at(11).toInt();
        stockDailyData.adjClose = strList.at(7).toFloat();
        if (0 == stockDailyData.volume)
            stockDailyData.close    = strList.at(7).toFloat();
        else
            stockDailyData.close    = strList.at(3).toFloat();
        return true;
    }
    return false;
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
        qDebug() << STR("### historyDailyDataReady -> %1 already exist ###").arg(stockCode);
        emit historyDailyDataReady(stockCode);
    } else {
        removePendingDelete();

        stockHistoryList.insert(stockCode, nullptr);    //nullptr在线程中被更新
        historyDailyDataProcessThread *thread = new historyDailyDataProcessThread(stockCode, this);
        threads.insert(stockCode, thread);
        connect(thread, SIGNAL(processFinish(QString)), this, SLOT(oneHistoryDailyDataInserted(QString)));
        thread->start();
    }
    qDebug() << STR("myStockHistoryData::insertStockHistory stockHistoryList.count:%1, pendingRemoveStock:%2")
                .arg(stockHistoryList.count()).arg(pendingRemoveStock.count());
}
void myStockHistoryData::deleteStockHistory(const QString &stockCode) {
    pendingRemoveStock.append(stockCode);
    removePendingDelete();
    qDebug() << STR("myStockHistoryData::deleteStockHistory stockHistoryList.count:%1, pendingRemoveStock:%2")
                .arg(stockHistoryList.count()).arg(pendingRemoveStock.count());
}

void myStockHistoryData::oneHistoryDailyDataInserted(const QString stockCode) {
    historyDailyDataProcessThread *thread = threads.value(stockCode);
    disconnect(thread, SIGNAL(processFinish(QString)), this, SLOT(oneHistoryDailyDataInserted(QString)));
    thread->deleteLater();
    threads.remove(stockCode);

    qDebug() << STR("### historyDailyDataReady -> %1 inserted ###").arg(stockCode);
    emit historyDailyDataReady(stockCode);
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

    QList<myStockDailyData> *data = stockHistoryList[stockCode];
    foreach(const myStockDailyData &dailyData, *data) {
        if (dailyData.datetime <= dateTime) {
            stockDailyData = dailyData;
            return true;
        } else if (dailyData.datetime > dateTime) {
            continue;
        } else {}
    }

    return false;
}
