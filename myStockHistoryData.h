#ifndef MYSTOCKHISTORYDATA_H
#define MYSTOCKHISTORYDATA_H

#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QString>
#include <QTimer>
#include <QDateTime>
#include <QList>

// http://table.finance.yahoo.com/table.csv?s=000001.sz
class myStockHistoryData : public QObject
{
    Q_OBJECT
/// Singleton
public:
    static myStockHistoryData *getInstance() {
        if (instance)
            return instance;
        else
            return initial();
    }
    static myStockHistoryData *initial() {
        if (!instance)
            instance = new myStockHistoryData();
        return instance;
    }
private:
    myStockHistoryData();
    static myStockHistoryData *instance;
    class CGarbo            //它的唯一工作就是在析构函数中删除CSingleton的实例
    {
    public:
        ~CGarbo() {
            if(myStockHistoryData::instance) {
                delete myStockHistoryData::instance;
                myStockHistoryData::instance = nullptr;
            }
        }
    };
    static CGarbo Garbo;

/// CLASS PART
public:
    struct myStockDailyData {
        QDateTime datetime;
        float open;
        float high;
        float low;
        float close;
        int   volume;
        float adjClose;
    };

public:
    ~myStockHistoryData();
    void getStockHistory(QString stockCode);

private:
    QStringList stockList;
    QList<myStockDailyData> stockHistoryList;

    QNetworkAccessManager *manager;
    QNetworkRequest ntRequest;
    QNetworkReply *reply;
    QTimer *replyTimeout;

private slots:
    void replyFinished(QNetworkReply* data);
    void handleTimeout();
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onReadyRead();
};

#endif // MYSTOCKHISTORYDATA_H
