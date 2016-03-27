#include "myStockPrice.h"
#include <QRegExp>

#include <QtDebug>

myStockPrice::myStockPrice() : manager (nullptr), isInitialized(false)
{
    manager = new QNetworkAccessManager();
    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));
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
    ntRequest.setUrl(QUrl(urlSina));
    manager->get(ntRequest);
}


void myStockPrice::replyFinished(QNetworkReply* data) {
    QByteArray stockPrice = data->readLine();
    isInitialized = false;
    stockPriceRt.clear();

    while (!stockPrice.isNull()) {
        QString priceData = QString::fromLocal8Bit(stockPrice);
        qDebug() << priceData;
        sinaRealTimeData sinaData;
        QStringList parts = priceData.split(",");
        QStringList codeName = parts.at(0).split("=");
        QStringList codeGet = codeName.at(0).split("_");
        QString code = codeGet.at(2);
        sinaData.code = code.insert(2, ".");
        sinaData.name = codeName.at(1);
        sinaData.open = parts.at(1).toDouble();
        sinaData.lastClose = parts.at(2).toDouble();
        sinaData.price = parts.at(3).toDouble();
        sinaData.top = parts.at(4).toDouble();
        sinaData.buttom = parts.at(5).toDouble();

        sinaData.buy1_1  = parts.at(6).toDouble();
        sinaData.sell1_1 = parts.at(7).toDouble();

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
        sinaData.sell1Stock = parts.at(20).toInt();
        sinaData.sell1Price = parts.at(21).toDouble();
        sinaData.sell2Stock = parts.at(22).toInt();
        sinaData.sell2Price = parts.at(23).toDouble();
        sinaData.sell3Stock = parts.at(24).toInt();
        sinaData.sell3Price = parts.at(25).toDouble();
        sinaData.sell4Stock = parts.at(26).toInt();
        sinaData.sell4Price = parts.at(27).toDouble();
        sinaData.sell5Stock = parts.at(28).toInt();
        sinaData.sell5Price = parts.at(29).toDouble();

        sinaData.time = QDateTime::fromString(parts.at(30)+" "+parts.at(31), "yyyy-MM-dd hh:mm:ss");
        sinaData.type = parts.at(32).toInt();

        stockPriceRt.insert(sinaData.code, sinaData);
        stockPrice = data->readLine();
    }
    // 通知Model刷新数据
    isInitialized = true;
    emit updatePriceFinish();
}
