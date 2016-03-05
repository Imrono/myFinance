#ifndef MYSTOCKCODENAME_H
#define MYSTOCKCODENAME_H

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>

#include <QtCore/QFile>

class myStockCodeName
{
    Q_OBJECT
public:
    myStockCodeName();
    ~myStockCodeName();

    void getStockCode();
    void getStockAbbreviation();


private:
    QNetworkAccessManager *manager;
    QNetworkRequest ntRequest;
    enum E_RequestTpye {
        REQUEST_CODE = 0
    };
    E_RequestTpye requestType;

private slots:
    void replyFinished(QNetworkReply* data);
};

#endif // MYSTOCKCODENAME_H
