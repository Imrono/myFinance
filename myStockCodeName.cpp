#include "myStockCodeName.h"
#include <QtCore/QFile>
#include <QtCore/QByteArray>
#include <QtCore/QFileInfo>

#include <QRegExp>

#include <QtDebug>
myStockCodeName::myStockCodeName()
    : manager(nullptr), ntRequest(QUrl("")),
      isInitialed(false), CodeDataFile("stockCodeData.txt")
{
    manager = new QNetworkAccessManager();
    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));

    if (QFileInfo::exists(CodeDataFile)) {
        analyzeStockCode(CodeDataFile);
    }
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
        QFile file(CodeDataFile);
        if (!file.open(QIODevice::WriteOnly|QIODevice::Text)) {
            qDebug() << "无法创建文件";
            return;
        }
        QTextStream toFile(&file);
        toFile << codeData;
        toFile.flush();
        file.close();
        analyzeStockCode(CodeDataFile);
        qDebug() << requestType << "finish";
        break;
        }
    default:
        break;
    }
}

void myStockCodeName::getStockCode() {
    QFileInfo info(CodeDataFile);
    QDateTime time = QDateTime::currentDateTime();
    QDateTime fileCreateTime = info.lastModified();
    if (!info.exists() || time.date().toJulianDay() - fileCreateTime.date().toJulianDay() >= 1) {
        ntRequest.setUrl(QUrl("http://quote.eastmoney.com/stocklist.html"));
        requestType = E_RequestTpye::REQUEST_CODE;
        manager->get(ntRequest);
    }
}

void myStockCodeName::getStockAbbreviation() {

}

void myStockCodeName::analyzeStockCode(QString fileName) {
    QString allCodeBeg = "<li><a target=\"_blank\" href=\"http://quote.eastmoney.com/";
    QString allCodeMid = ".html\">";
    QString allCodeEnd = "</a></li>";

    QString pattern = QString("%1(.*)%2(.*)%3").arg(allCodeBeg).arg(allCodeMid).arg(allCodeEnd);
    QRegExp rx(pattern);

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "无法创建文件";
        return;
    }
    QTextStream stream(&file);

    codeName.clear();
    isInitialed = false;
    QString line;
    QRegExp rxCode("([a-zA-Z]*)([0-9]*)");
    QRegExp rxName("[(][0-9]*[)]");
    while (!stream.atEnd()) {
        line = stream.readLine();
        int pos = line.indexOf(rx);
        if (pos >= 0) {
            QString code;
            QString name;
            QString a = rx.cap(1);
            QString b = rx.cap(2);
            qDebug() << a << b;
            pos = a.indexOf(rxCode);
            if (pos >= 0) {
                QString code1 = rxCode.cap(1);
                QString code2 = rxCode.cap(2);
                code = QString("%1.%2").arg(code1).arg(code2);
            } else { continue; }
            name = b.remove(rxName);
            codeName.insert(code, name);
        } else { continue; }
    }
    file.close();
    isInitialed = true;
    qDebug() << codeName.count();
}
