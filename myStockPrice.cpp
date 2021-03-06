﻿#include "myStockPrice.h"
#include <QRegExp>

#include <QtDebug>
#include <QMessageBox>
#include <QtNetwork/QNetworkRequest>

myStockPrice::myStockPrice() : manager (nullptr), isInitialized(false)
  , replyTimeout(nullptr), reply(nullptr)
{
    manager = new QNetworkAccessManager();
    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));
    replyTimeout = new QTimer(this);
    connect(replyTimeout, SIGNAL(timeout()), this, SLOT(handleTimeout()));
    replyTimeout->setSingleShot(true);
}
myStockPrice::~myStockPrice()
{
    if (nullptr != manager) {
        delete manager;
        manager = nullptr;
    }
}

void myStockPrice::getStockPrice(const QStringList &list) {
    stockList = list;
    QString urlSina;
    QString prefix = "http://hq.sinajs.cn/list=";
    urlSina = prefix;
    for (QStringList::iterator it = stockList.begin(); it != stockList.end(); ++it) {
        QString stockCode = *it;
        if (stockCode.contains(".")) {
            stockCode.remove(".");
            urlSina += stockCode;
            urlSina += ",";
        }
    }
    reply = manager->get(QNetworkRequest(QUrl(urlSina)));
    MY_DEBUG_URL(urlSina);
    //replyTimeout->start(6000);
    //qDebug() << "### start TIMER 6s for stock price get ###";
}


void myStockPrice::replyFinished(QNetworkReply* data) {
    replyTimeout->stop();

    QByteArray stockPrice = data->readLine();
    isInitialized = false;
    stockPriceRt.clear();

    while (!stockPrice.isNull()) {
        QString priceData = STR(stockPrice);
        qDebug() << priceData;
        sinaRealTimeData sinaData;
        QStringList parts = priceData.split(",");
        QStringList codeName = parts.at(0).split("=");
        QStringList codeGet = codeName.at(0).split("_");
        QString code = codeGet.at(2);
        if (1 == parts.count()) {
            MY_DEBUG_ERROR(code.append("'s data is invalid"));
            stockPrice = data->readLine();
            continue;
        }
        sinaData.code = code.insert(2, ".");
        sinaData.name = codeName.at(1);
        sinaData.open = parts.at(1).toDouble();
        sinaData.lastClose = parts.at(2).toDouble();
        sinaData.price = parts.at(3).toDouble();
        sinaData.top = parts.at(4).toDouble();
        sinaData.buttom = parts.at(5).toDouble();

        sinaData.buy1_1  = parts.at(6).toDouble();
        sinaData.bid1_1 = parts.at(7).toDouble();

        sinaData.stockVolume = parts.at(8).toInt();
        sinaData.moneyVolume = parts.at(9).toInt();

        sinaData.buy1Stock = parts.at(10).toInt();
        sinaData.buy1Price = parts.at(11).toDouble();
        sinaData.buy2Stock = parts.at(12).toInt();
        sinaData.buy2Price = parts.at(13).toDouble();
        sinaData.buy3Stock = parts.at(14).toInt();
        sinaData.buy3Price = parts.at(15).toDouble();
        sinaData.buy4Stock = parts.at(16).toInt();
        sinaData.buy4Price = parts.at(17).toDouble();
        sinaData.buy5Stock = parts.at(18).toInt();
        sinaData.buy5Price = parts.at(19).toDouble();
        sinaData.bid1Stock = parts.at(20).toInt();
        sinaData.bid1Price = parts.at(21).toDouble();
        sinaData.bid2Stock = parts.at(22).toInt();
        sinaData.bid2Price = parts.at(23).toDouble();
        sinaData.bid3Stock = parts.at(24).toInt();
        sinaData.bid3Price = parts.at(25).toDouble();
        sinaData.bid4Stock = parts.at(26).toInt();
        sinaData.bid4Price = parts.at(27).toDouble();
        sinaData.bid5Stock = parts.at(28).toInt();
        sinaData.bid5Price = parts.at(29).toDouble();

        sinaData.time = QDateTime::fromString(parts.at(30)+" "+parts.at(31), "yyyy-MM-dd hh:mm:ss");
        sinaData.type = parts.at(32).toInt();

        stockPriceRt.insert(sinaData.code, sinaData);
        stockPrice = data->readLine();
    }
    // 通知Model刷新数据
    isInitialized = true;
    emit updatePriceFinish();
}
void myStockPrice::handleTimeout() {
    qDebug() << "### stock price get TIMER timeout!! ###";
    replyTimeout->stop();
    reply->abort();
    QMessageBox::warning(nullptr, "timeout", STR("获取股票价格超时"), QMessageBox::Ok, QMessageBox::Ok);
}
