#include "myStockCodeName.h"
#include <QtCore/QFile>
#include <QtCore/QByteArray>

#include <QRegExp>

#include <QtDebug>
myStockCodeName::myStockCodeName()
    : manager(nullptr), ntRequest(QUrl(""))
{
    manager = new QNetworkAccessManager();
    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));
}

myStockCodeName::~myStockCodeName() {
    if (manager)
        delete manager;
}

void myStockCodeName::replyFinished(QNetworkReply* data) {
    qDebug() << requestType << "begin";
    switch(requestType) {
    case E_RequestTpye::REQUEST_CODE: {
        QByteArray codeDataArray = data->readAll();
        QString codeData = QString::fromLocal8Bit(codeDataArray);
        QFile file("stockCodeData.txt");
        if (!file.open(QIODevice::WriteOnly|QIODevice::Text)) {
            qDebug() << "无法创建文件";
            return;
        }
        QTextStream toFile(&file);
        toFile << codeData;
        toFile.flush();
        file.close();
        analyzeStockCode("stockCodeData.txt");
        qDebug() << requestType << "finish";
        break;
        }
    default:
        break;
    }
}

void myStockCodeName::getStockCode() {
    ntRequest.setUrl(QUrl("http://quote.eastmoney.com/stocklist.html"));
    requestType = E_RequestTpye::REQUEST_CODE;
    manager->get(ntRequest);
}

void myStockCodeName::getStockAbbreviation() {

}

void myStockCodeName::analyzeStockCode(QString fileName) {
    QString allCodeBeg = "<li><a target=\"_blank\" href=\"http://quote.eastmoney.com/";
    QString allCodeMid = ".html\">";
    QString allCodeEnd = "</a></li>";

    QString pattern("%1(.*)%2=(.*)%3");
    QRegExp reg(pattern);
    QString line;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "无法创建文件";
        return;
    }

    QTextStream stream(&file);
    while (!stream.atEnd()) {
        line = stream.readLine();
    }
    file.close();
}
