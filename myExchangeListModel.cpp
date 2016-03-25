#include "myExchangeListModel.h"
#include "myFinanceDatabase.h"
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlError>
#include <QtSql/QSqlRecord>

#include <QDebug>

myExchangeListModel::myExchangeListModel() {
    if (!myFinanceDatabase::isConnected) {
        myFinanceDatabase::connectDB();
    }
    initial();
}
myExchangeListModel::~myExchangeListModel() {

}

bool myExchangeListModel::doExchange(const exchangeData data) {
    QString exchangeTime = data.time.toString(("yyyy-mm-dd hh:mm:ss"));
    QString exchangeType = data.type;
    QSqlQuery query;
    // 1 update database
    // "�ʲ��仯"�� CHANGE
    QString execWord = QString::fromLocal8Bit("INSERT INTO �ʲ��仯 "
                                      "VALUES (null, '%1', '%2', '%3', %4, '%5', '%6', '%7', %8, %9)")
            .arg(exchangeTime).arg(exchangeType).arg(data.account1).arg(data.money)
            .arg(data.account2).arg(data.code).arg(data.name).arg(data.price).arg(data.amount);
    qDebug() << execWord;

    if(!query.exec(execWord)) {
        qDebug() << query.lastError().text();
        return false;
    }
    // 2 ����list��ˢ��
    return initial();
}

bool myExchangeListModel::initial() {
    list.clear();
    QSqlQuery query;
    int numRows = 0;
    ///�����ʲ��ʻ�����
    if(query.exec(QString::fromLocal8Bit("select * from �ʲ��仯"))) {
        if(myFinanceDatabase::db.driver()->hasFeature(QSqlDriver::QuerySize)){
            numRows = query.size();
        } else {
            query.last();
            numRows = query.at() + 1;
        }
        int i = 0;
        while(query.next() && i < numRows) { // ��λ�������һ����¼
            exchangeData tmpExchange;
            tmpExchange.id       = query.value(0).toInt();
            tmpExchange.time     = QDateTime::fromString(query.value(1).toString(), "yyyy-MM-ddThh:mm:ss");
            tmpExchange.type     = query.value(2).toString();
            tmpExchange.account1 = query.value(3).toString();
            tmpExchange.money    = query.value(4).toInt();
            tmpExchange.account2 = query.value(5).toString();
            tmpExchange.code     = query.value(6).toString();
            tmpExchange.name     = query.value(7).toString();
            tmpExchange.price    = query.value(8).toDouble();
            tmpExchange.amount   = query.value(9).toInt();

            i ++;
            QString exchangeStr;
            if (CASH == tmpExchange.code && 1 == tmpExchange.amount) {
                if (tmpExchange.price + tmpExchange.money > 0.0001f)
                    qDebug() << "[ת��]" << tmpExchange.account1 << " " << tmpExchange.money
                             << "!="    << tmpExchange.account2 << " " << tmpExchange.price;
                exchangeStr = QString::fromLocal8Bit("[%1]%2->%3(��%4)")
                        .arg(tmpExchange.type)
                        .arg(tmpExchange.account1).arg(tmpExchange.account2).arg(tmpExchange.price);
            } else {
                exchangeStr = QString::fromLocal8Bit("[%1]%2(��%3) - %4@%5(%6*%7)")
                        .arg(tmpExchange.type)
                        .arg(tmpExchange.account1).arg(tmpExchange.money)
                        .arg(tmpExchange.name).arg(tmpExchange.account2)
                        .arg(tmpExchange.amount).arg(tmpExchange.price);
            }
            list.append(exchangeStr);
        }
        qDebug() << "num of exchange data : " << i;
    } else { // �����ѯʧ�ܣ�������ķ����õ��������ݿⷵ�ص�ԭ��
        qDebug() << "Fetch Account Data to MySql error: " << query.lastError().text();
        return nullptr;
    }
    setStringList(list);
    return true;
}