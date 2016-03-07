#ifndef MYSTOCKCODENAME_H
#define MYSTOCKCODENAME_H

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

#include <QtCore/QFile>
#include <QtCore/QMap>

class myStockCodeName : public QObject
{
    Q_OBJECT
public:
    myStockCodeName();
    ~myStockCodeName();

    void getStockCode();
    void getStockAbbreviation();
    bool getIsInitialed() { return isInitialed;}

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

    void analyzeStockCode(QString fileName);

private slots:
    void replyFinished(QNetworkReply* data);
};

#endif // MYSTOCKCODENAME_H
