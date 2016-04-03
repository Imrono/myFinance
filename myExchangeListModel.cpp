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

bool myExchangeListModel::doExchange(const myExchangeData &data) {
    QString exchangeTime = data.time.toString(("yyyy-mm-dd hh:mm:ss"));
    QString exchangeType = data.type;
    QSqlQuery query;
    QString execWord;

    // 1 update database, "资产变化"表 CHANGE
    execWord = QString::fromLocal8Bit("SELECT * FROM 资产变化 WHERE id=%1").arg(data.id);
    qDebug() << execWord;
    if(query.exec(execWord)) {
        if (1 == query.size()) {        //UPDATE
            execWord = QString::fromLocal8Bit("UPDATE 资产变化 "
                                              "SET ='%1', ='%2', ='%3', =%4, ='%5', ='%6', ='%7', =%8, =%9) "
                                              "WHERE id=%10")
                    .arg(exchangeTime).arg(exchangeType).arg(data.account1).arg(data.money)
                    .arg(data.account2).arg(data.code).arg(data.name).arg(data.price).arg(data.amount).arg(data.id);
            qDebug() << execWord;

            if(!query.exec(execWord)) {
                qDebug() << query.lastError().text();
                return false;
            }
        } else if (0 == query.size()) { //INSERT
            execWord = QString::fromLocal8Bit("INSERT INTO 资产变化 "
                                              "VALUES (null, '%1', '%2', '%3', %4, '%5', '%6', '%7', %8, %9)")
                    .arg(exchangeTime).arg(exchangeType).arg(data.account1).arg(data.money)
                    .arg(data.account2).arg(data.code).arg(data.name).arg(data.price).arg(data.amount);
            qDebug() << execWord;

            if(!query.exec(execWord)) {
                qDebug() << query.lastError().text();
                return false;
            }
        } else {
            return false;
        }
    } else {
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
            if (MY_CASH == tmpExchange.code && 1 == tmpExchange.amount) {
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

void myExchangeListModel::coordinatorModifyExchange(myExchangeData &originData, myExchangeData &targetData, int &changeIdx) {
    changeIdx = NO_DO_EXCHANGE;
    myExchangeData tmpOrigin, tmpTarget;
    if (originData.account1 != targetData.account1) {
        changeIdx |= ORIG_ACCOUNT_1;
        changeIdx |= TARG_ACCOUNT_1;
    }
    if (originData.account2 != targetData.account2) {
        changeIdx |= ORIG_ACCOUNT_2;
        changeIdx |= TARG_ACCOUNT_2;
    }
    if (qAbs(originData.money - targetData.money) < MONEY_EPS) {
        changeIdx |= ORIG_ACCOUNT_1;
    }
    if (   originData.code != targetData.code
        || originData.name != targetData.name
        || (qAbs(originData.price - targetData.price) < MONEY_EPS)
        || originData.amount != targetData.amount) {
        changeIdx |= ORIG_ACCOUNT_2;
    }
    if (   originData.time != targetData.time
        || originData.type != targetData.type) {
        changeIdx |= OTHER_EXCHANGE;
        tmpOrigin.time = targetData.time;
        tmpTarget.time = targetData.time;
        tmpOrigin.type = targetData.type;
        tmpTarget.type = targetData.type;
    }

    if (changeIdx & ORIG_ACCOUNT_1) {
        tmpOrigin.account1 = originData.account1;
        if (changeIdx & TARG_ACCOUNT_1) {
            tmpTarget.account1 = targetData.account1;
            tmpOrigin.money = -originData.money;
            tmpTarget.money = targetData.money;
        } else {
            tmpOrigin.money = targetData.money - originData.money;
            tmpTarget.money = 0.0f;
        }
    }
    if (changeIdx & ORIG_ACCOUNT_2) {
        tmpOrigin.account2 = originData.account2;
        tmpOrigin.code = originData.code;
        tmpOrigin.name = originData.name;

        if (changeIdx & TARG_ACCOUNT_2) {
            tmpTarget.account2 = targetData.account2;

            if (originData.code == MY_CASH && 1 == originData.amount) {
                tmpOrigin.amount = originData.amount;
                tmpOrigin.price = -originData.price;
            } else {
                tmpOrigin.amount = -originData.amount;
                tmpOrigin.price = originData.price;
            }

            tmpTarget.code = targetData.code;
            tmpTarget.name = targetData.name;
            tmpTarget.amount = targetData.amount;
            tmpTarget.price = targetData.price;
        } else {
            if (originData.code == MY_CASH && 1 == originData.amount) {
                if (targetData.code == MY_CASH && 1 == targetData.amount) {
                    tmpOrigin.amount = 1;
                    tmpOrigin.price = targetData.price - originData.price;
                } else {
                    changeIdx |= TARG_ACCOUNT_1;

                    tmpOrigin.amount = originData.amount;
                    tmpOrigin.price = -originData.price;

                    tmpTarget.code = targetData.code;
                    tmpTarget.name = targetData.name;
                    tmpTarget.price = targetData.price;
                    tmpTarget.amount = targetData.amount;
                }
            } else {
                if (targetData.code == MY_CASH && 1 == targetData.amount) {
                    changeIdx |= TARG_ACCOUNT_1;

                    tmpOrigin.amount = -originData.amount;
                    tmpOrigin.price = originData.price;

                    tmpTarget.code = targetData.code;
                    tmpTarget.name = targetData.name;
                    tmpTarget.price = targetData.price;
                    tmpTarget.amount = targetData.amount;
                } else {
                    tmpOrigin.amount = targetData.amount - originData.amount;
                    tmpOrigin.price = originData.price;
                }
            }

        }
    }

    originData = tmpOrigin;
    targetData = tmpTarget;
}
