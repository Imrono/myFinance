#ifndef MYSTOCKHISTORYDATA_H
#define MYSTOCKHISTORYDATA_H
#include "myGlobal.h"

#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QString>
#include <QTimer>
#include <QDateTime>
#include <QList>
#include <QMap>
#include <QThread>

struct myStockDailyData {
    QDateTime datetime;
    float open;
    float high;
    float low;
    float close;
    int   volume;
    float adjClose;
};

class myStockHistoryData;
class historyDailyDataProcessThread : public QThread {
    Q_OBJECT
public:
    historyDailyDataProcessThread(const QString &stockCode, myStockHistoryData* parent);
    ~historyDailyDataProcessThread();

signals:
    void processFinish(const QString stockCode);
private slots:
    void stockHistoryFinished(QNetworkReply *reply);
    void stockReadyRead();
    void stockDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);

protected:
    void run() Q_DECL_OVERRIDE;

private:
    myStockHistoryData* parent;
    const QString stockCode;
    QNetworkReply *reply;

    static QString stockCode2YahooStyle(const QString &stockCode);
    static QString stockUrlYahoo(const QString &stockCode);
    static bool stockDecodeYahoo(QByteArray &lineData, myStockDailyData &dailyData);
    static QString stockCode2NetEaseStyle(const QString &stockCode);
    static QString stockUrlNetEase(const QString &stockCode);
    static bool stockDecodeNetEase(QByteArray &lineData, myStockDailyData &dailyData);
};

// http://table.finance.yahoo.com/table.csv?s=000001.sz
class myStockHistoryData : public QObject
{
    Q_OBJECT
    friend class historyDailyDataProcessThread;
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
    ~myStockHistoryData();
    void insertStockHistory(const QString &stockCode);
    void deleteStockHistory(const QString &stockCode);
    bool getStockDailyData(const QString &stockCode, const QDateTime dateTime, myStockDailyData &stockDailyData);
    void removePendingDelete();
private:
    QStringList stockList;
    QString lastStockCode;
    QMap<QString, QList<myStockDailyData> *> stockHistoryList;
    QList<QString> pendingRemoveStock;
    const int maxNumOfHistories;
    QMap<QString, historyDailyDataProcessThread *> threads;

signals:
    void historyDailyDataReady(const QString stockCode);
private slots:
    void oneHistoryDailyDataInserted(const QString stockCode);
};

#endif // MYSTOCKHISTORYDATA_H
