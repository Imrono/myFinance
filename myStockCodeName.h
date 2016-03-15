#ifndef MYSTOCKCODENAME_H
#define MYSTOCKCODENAME_H

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

#include <QtCore/QFile>
#include <QtCore/QMap>

#include <QThread>
class myStockCodeName;
class codeDataProcessThread : public QThread {
    Q_OBJECT
public:
    friend class myStockCodeName;
    codeDataProcessThread(myStockCodeName* parent);
    ~codeDataProcessThread();

signals:
    void processFinish();
protected:
    void run() Q_DECL_OVERRIDE;

private:
    myStockCodeName* parent;
};

class myStockCodeName : public QObject
{
    Q_OBJECT
public:
    friend class codeDataProcessThread;
    myStockCodeName();
    ~myStockCodeName();

    void getStockCode();
    void getStockAbbreviation();
    bool getIsInitialed() { return isInitialed;}
    codeDataProcessThread &getThread() {
        return thread;
    }
    bool getIsDataReady() {return isDataReady;}
    QMap <QString, QString> codeName;


private:
    QNetworkAccessManager *manager;
    QNetworkRequest ntRequest;
    enum E_RequestTpye {
        REQUEST_CODE = 0
    };
    E_RequestTpye requestType;
    bool isInitialed;
    QString CodeDataFile;

    codeDataProcessThread thread;
    bool isDataReady;
    void analyzeStockCode(QString fileName);

private slots:
    void replyFinished(QNetworkReply* data);
    void threadProcessFinish();
signals:
    void codeDataReady();
};

#endif // MYSTOCKCODENAME_H
