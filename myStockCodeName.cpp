#include "myStockCodeName.h"
#include <QtCore/QFile>
#include <QtCore/QByteArray>

#include <QtDebug>
myStockCodeName::myStockCodeName()
    : manager(nullptr), ntRequest(QUrl(""))
{
    allCodeStart = "<li><a target=\"_blank\" href=\"http://quote.eastmoney.com/";
    allCodeMid = ".html\">";
    allCodeEnd = "</a></li>";

    manager = new QNetworkAccessManager();
    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));
}

myStockCodeName::~myStockCodeName() {
    if (manager)
        delete manager;
}

void myStockCodeName::replyFinished(QNetworkReply* data) {
    switch(requestType) {
    case E_RequestTpye::REQUEST_CODE: {
        QByteArray codeData = data->readAll();
        QFile file("stockCodeData.txt");
        if (!file.open(QIODevice::WriteOnly|QIODevice::Text)) {
            qDebug() << "无法创建文件";
            return;
        }
        QTextStream toFile(&file);
        toFile << codeData;
        toFile.flush();
        file.close();
        break;
        }
    default:
        break;
    }
}

void myStockCodeName::getStockCode() {
    ntRequest.setUrl(QUrl("http://quote.eastmoney.com/stocklist.html"));
    manager->get(ntRequest);
}

void myStockCodeName::getStockAbbreviation() {

}
