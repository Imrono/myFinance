#include "myStockHistoryData.h"
#include "myDatabaseDatatype.h"

#include <QByteArray>
#include <QDebug>
#include <QMessageBox>

myStockHistoryData *myStockHistoryData::instance = nullptr;
myStockHistoryData::myStockHistoryData()
{
    manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));

    replyTimeout = new QTimer(this);
    connect(replyTimeout, SIGNAL(timeout()), this, SLOT(handleTimeout()));
    replyTimeout->setSingleShot(true);
}
myStockHistoryData::~myStockHistoryData() {
}

void myStockHistoryData::getStockHistory(QString stockCode) {
    QString urlYahooHistory;
    QString prefix = STR("http://table.finance.yahoo.com/table.csv?s=");
    urlYahooHistory = prefix;

    QString preStr = stockCode.left(3);
    if ("sh." == preStr) {
        stockCode.remove(0, 3);
        stockCode.append(STR(".ss"));
    } else if ("sz." == preStr) {
        stockCode.remove(0, 3);
        stockCode.append(STR(".sz"));
    } else {}
    urlYahooHistory = prefix + stockCode;

    ntRequest.setUrl(QUrl(urlYahooHistory));
    reply = manager->get(ntRequest);
    MY_DEBUG_URL(ntRequest.url().toString());
    connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(onDownloadProgress(qint64, qint64)));
    connect(reply, SIGNAL(readyRead()),this, SLOT(onReadyRead()));

    //replyTimeout->start(6000);
    //qDebug() << "### start TIMER 6s for stock price get ###";
}

void myStockHistoryData::replyFinished(QNetworkReply* data) {
    replyTimeout->stop();
    stockHistoryList.clear();

    if (reply->error()) {
        qDebug() << "failed : " << reply->errorString();
        QMessageBox::information(NULL, STR("reply error"), reply->errorString());
    }

    QByteArray lineData = data->readLine();
    unsigned historyCount = 0;
    while (!lineData.isNull()) {
        //qDebug() << stockDailyData;
        lineData = data->readLine();
        QList<QByteArray> strList = lineData.split(',');

        if (7 == strList.count()) {
            myStockDailyData stockDailyData;
            stockDailyData.datetime = QDateTime::fromString(strList.at(0), "yyyy-MM-dd");
            stockDailyData.open     = strList.at(1).toFloat();
            stockDailyData.high     = strList.at(2).toFloat();
            stockDailyData.low      = strList.at(3).toFloat();
            stockDailyData.close    = strList.at(4).toFloat();
            stockDailyData.volume   = strList.at(5).toInt();
            stockDailyData.adjClose = strList.at(6).toFloat();

            stockHistoryList.append(stockDailyData);
            historyCount ++;
        }
    }
    qDebug() << "myStockHistoryData::replyFinished with historyCount:" << historyCount;
}
void myStockHistoryData::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal) {
    qDebug() << STR("### myStockHistoryData::onDownloadProgress -> bytesReceived:%1, bytesTotal:%2 ###").arg(bytesReceived).arg(bytesTotal);
}
void myStockHistoryData::onReadyRead() {
    qDebug() << "### myStockHistoryData::onReadyRead ###";
}

void myStockHistoryData::handleTimeout() {
    qDebug() << "### myStockHistoryData get TIMER timeout!! ###";
    replyTimeout->stop();
    reply->abort();
    QMessageBox::warning(nullptr, "timeout", STR("获取股票历史价格超时"), QMessageBox::Ok, QMessageBox::Ok);
}
