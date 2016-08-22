#ifndef MYSTOCKPRICE_H
#define MYSTOCKPRICE_H
#include "myGlobal.h"

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

#include <QObject>
#include <QMap>
#include <QTimer>

#include "myAssetNode.h"

struct sinaRealTimeData {
    QString code;
    QString name;

    float open;
    float lastClose;
    float price;
    float top;
    float buttom;

    float buy1_1;
    float bid1_1;

    int   stockVolume;
    float moneyVolume;

    int   buy1Stock;
    float buy1Price;
    int   buy2Stock;
    float buy2Price;
    int   buy3Stock;
    float buy3Price;
    int   buy4Stock;
    float buy4Price;
    int   buy5Stock;
    float buy5Price;
    int   bid1Stock;
    float bid1Price;
    int   bid2Stock;
    float bid2Price;
    int   bid3Stock;
    float bid3Price;
    int   bid4Stock;
    float bid4Price;
    int   bid5Stock;
    float bid5Price;

    QDateTime time;
    int type;
};

class myStockPrice : public QObject
{
    Q_OBJECT
public:
    myStockPrice();
    ~myStockPrice();

    void getStockPrice(const QStringList &list);

    QMap<QString, sinaRealTimeData> const *getStockPriceRt() const {
        return &stockPriceRt;
    }
    bool isInit() const {
        return isInitialized;
    }

signals:
    void updatePriceFinish();

private:
    QNetworkAccessManager *manager;
    QNetworkReply *reply;
    QTimer *replyTimeout;
    bool isInitialized;

    QStringList stockList;
    QMap<QString, sinaRealTimeData> stockPriceRt;

private slots:
    void replyFinished(QNetworkReply* data);
    void handleTimeout();

};

#endif // MYSTOCKPRICE_H
