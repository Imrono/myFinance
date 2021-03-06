#include "myStockCodeName.h"
#include "myDatabaseDatatype.h"
#include <QtCore/QFile>
#include <QtCore/QByteArray>
#include <QtCore/QFileInfo>
#include <QTextCodec>
#include <QRegExp>

#include <QtDebug>

codeDataProcessThread::codeDataProcessThread(myStockCodeName* parent) {
    this->parent = parent;
}
codeDataProcessThread::~codeDataProcessThread() {

}
void codeDataProcessThread::run() {
    qDebug() << "### analyzeStockCode run ###";
    mutex.lock();
    parent->analyzeStockCode(parent->CodeDataFile);
    emit processFinish();
    mutex.unlock();
}

myStockCodeName *myStockCodeName::instance = nullptr;
myStockCodeName::myStockCodeName()
    : manager(nullptr), ntRequest(QUrl("")), CodeDataFile("stockCodeData.txt"),
      thread(this), isDataReady(false)
{
    manager = new QNetworkAccessManager();
    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));
    connect(&thread, SIGNAL(processFinish()),
            this, SLOT(threadProcessFinish()));

    if (QFileInfo::exists(CodeDataFile)) {
        if (!isDataReady) {
            thread.start();
        } else {
            qDebug() << "doing Process";
        }
    }
}

myStockCodeName::~myStockCodeName() {
    if (manager) {
        delete manager;
        manager = nullptr;
    }
}

void myStockCodeName::replyFinished(QNetworkReply* data) {
    qDebug() << "requestType:" << requestType << "begin";
    switch(requestType) {
    case REQUEST_CODE: {
        QByteArray codeDataArray = data->readAll();
        QTextCodec *tc = QTextCodec::codecForName("GBK");
        QString codeData = tc->toUnicode(codeDataArray);
        QFile file(CodeDataFile);
        if (!file.open(QIODevice::WriteOnly|QIODevice::Text)) {
            qDebug() << "无法创建文件";
            return;
        }
        QTextStream toFile(&file);
        toFile << codeData;
        toFile.flush();
        file.close();

        if (!isDataReady) {
            thread.start();
        } else {
            qDebug() << "data is ready, no need to analyze stock code file";
        }

        qDebug() << "requestType:" << requestType << "finish";
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
        isDataReady = false;
        ntRequest.setUrl(QUrl("http://quote.eastmoney.com/stocklist.html"));
        requestType = REQUEST_CODE;
        manager->get(ntRequest);
    } else {
        isDataReady = true;
        emit codeDataReady();
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
        qDebug() << STR("无法创建文件");
        return;
    }
    QTextStream stream(&file);

    codeName.clear();
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
            //qDebug() << a << b;
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
    qDebug() << "myStockCodeName::analyzeStockCode=>codeName.count() =" << codeName.count();
}
void myStockCodeName::threadProcessFinish() {
    qDebug() << "threadProcessFinish";
    isDataReady = true;
    emit codeDataReady();
}

QString myStockCodeName::findNameFromCode(const QString &code) const {
    if (this->getIsDataReady()) {
        QMap<QString, QString>::const_iterator ii = codeName.find(code);
        if (ii != codeName.end() && ii.key() == code) {
            return ii.value();
        }
    }
    return QString("");
}
