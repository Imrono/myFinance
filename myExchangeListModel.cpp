#include "myExchangeListModel.h"
#include "myFinanceDatabase.h"
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlError>
#include <QtSql/QSqlRecord>

#include <QDebug>

myExchangeListModel::myExchangeListModel() : data(nullptr) {
    if (!myFinanceDatabase::isConnected) {
        myFinanceDatabase::connectDB();
    }
    initial();
}
myExchangeListModel::~myExchangeListModel() {
    if (nullptr != data) {
        delete []data;
        data = nullptr;
    }
}

bool myExchangeListModel::doExchange(const myExchangeData data) {
    QString exchangeTime = data.time.toString(("yyyy-mm-dd hh:mm:ss"));
    QString exchangeType = data.type;
    QSqlQuery query;
    // 1 update database
    // "资产变化"表 CHANGE
    QString execWord = QString::fromLocal8Bit("INSERT INTO 资产变化 "
                                      "VALUES (null, '%1', '%2', '%3', %4, '%5', '%6', '%7', %8, %9)")
            .arg(exchangeTime).arg(exchangeType).arg(data.account1).arg(data.money)
            .arg(data.account2).arg(data.code).arg(data.name).arg(data.price).arg(data.amount);
    qDebug() << execWord;

    if(!query.exec(execWord)) {
        qDebug() << query.lastError().text();
        return false;
    }
    // 2 更新list，刷新
    return initial();
}

bool myExchangeListModel::initial() {
    list.clear();
    if (nullptr != data) {
        delete []data;
        data = nullptr;
    }

    QSqlQuery query;
    int numRows = 0;
    ///读“资产帐户”表
    if(query.exec(QString::fromLocal8Bit("select * from 资产变化"))) {
        if(myFinanceDatabase::db.driver()->hasFeature(QSqlDriver::QuerySize)){
            numRows = query.size();
        } else {
            query.last();
            numRows = query.at() + 1;
        }
        data = new myExchangeData[numRows];
        int i = 0;
        while(query.next() && i < numRows) { // 定位结果到下一条记录
            myExchangeData tmpExchange;
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

            QString exchangeStr;
            if (CASH == tmpExchange.code && 1 == tmpExchange.amount) {
                if (tmpExchange.price + tmpExchange.money > 0.0001f)
                    qDebug() << "[转帐]" << tmpExchange.account1 << " " << tmpExchange.money
                             << "!="    << tmpExchange.account2 << " " << tmpExchange.price;
                exchangeStr = QString::fromLocal8Bit("[%1]%2->%3(￥%4)")
                        .arg(tmpExchange.type)
                        .arg(tmpExchange.account1).arg(tmpExchange.account2).arg(tmpExchange.price);
            } else {
                exchangeStr = QString::fromLocal8Bit("[%1]%2(￥%3) - %4@%5(%6*%7)")
                        .arg(tmpExchange.type)
                        .arg(tmpExchange.account1).arg(tmpExchange.money)
                        .arg(tmpExchange.name).arg(tmpExchange.account2)
                        .arg(tmpExchange.amount).arg(tmpExchange.price);
            }
            list.append(exchangeStr);
            data[i] = tmpExchange;
            i ++;
        }
        qDebug() << "num of exchange data : " << i;
    } else { // 如果查询失败，用下面的方法得到具体数据库返回的原因
        qDebug() << "Fetch Account Data to MySql error: " << query.lastError().text();
        return nullptr;
    }
    setStringList(list);
    return true;
}

myExchangeData myExchangeListModel::getDataFromRow(int row) {
    return data[row];
}

void myExchangeListModel::coordinatorModifyExchange(myExchangeData &originData, myExchangeData &targetData, int &type) {
    type = NO_CHANGE;
    if (originData.account1 != targetData.account1)
        type |= ACCOUNT1_CHANGE;
    if (originData.account2 != targetData.account2)
        type |= ACCOUNT2_CHANGE;
    if (originData.money != targetData.money)
        type |= MONEY_CHANGE;
    if (   originData.code != targetData.code
        || originData.name != targetData.name
        || (originData.price - targetData.price < MONEY_EPS)
        || originData.amount != targetData.amount) {
        type |=ASSET_CHANGE;
    }
    if (   originData.time != targetData.time
        || originData.type != targetData.type) {
        type |=OTHER_CHANGE;
    }
}
