#ifndef MYSTOCKCODENAME_H
#define MYSTOCKCODENAME_H

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

#include <QtCore/QFile>
#include <QtCore/QMap>

#include <QThread>
#include <QMutex>
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
    QMutex mutex;
};

///
/// \brief The myStockCodeName class
/// 在myFinanceMainWindow中，用于更新ui的状态
/// 在myFinanceExchangeWindow中，用于从code到name的推导
///
class myStockCodeName : public QObject
{
    Q_OBJECT
public:
    friend class codeDataProcessThread;
    ~myStockCodeName();

    void getStockCode();
    void getStockAbbreviation();
    bool getIsInitialed() const { return isInitialed;}
    codeDataProcessThread &getThread() {
        return thread;
    }
    bool getIsDataReady() {return isDataReady;}
    QMap <QString, QString> codeName;

    QString findNameFromCode(const QString &code) const;

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

/// Singleton
private:
    myStockCodeName();
    static myStockCodeName *instance;
    class CGarbo            //它的唯一工作就是在析构函数中删除CSingleton的实例
    {
    public:
        ~CGarbo()
        {
            if(myStockCodeName::instance)
            delete myStockCodeName::instance;
        }
    };
    static CGarbo Garbo;    //定义一个静态成员变量，程序结束时，系统会自动调用它的析构函数
public:
    static myStockCodeName *getInstance() {
        if (!instance)
            return instance;
        else
            return initial();
    }
    static myStockCodeName *initial() {
        if (instance)
            return instance;
        else {
            instance = new myStockCodeName();
            return instance;
        }
    }

};

#endif // MYSTOCKCODENAME_H
