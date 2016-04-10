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

bool myExchangeListModel::doExchange(const myExchangeData &exchangeData, bool isDelete) {
    QSqlQuery query;
    QString execWord;
    if (isDelete) {
        execWord = QString::fromLocal8Bit("DELETE FROM 资产变化 WHERE id=%1").arg(exchangeData.id);
        qDebug() << execWord;
        if(!query.exec(execWord)) {
            qDebug() << query.lastError().text();
            return false;
        }
    } else {
        QString exchangeTime = exchangeData.time.toString(("yyyy-MM-dd hh:mm:ss"));
        QString exchangeType = exchangeData.type;
        // 1 update database, "资产变化"表 CHANGE
        execWord = QString::fromLocal8Bit("SELECT * FROM 资产变化 WHERE id=%1").arg(exchangeData.id);
        qDebug() << execWord;
        if(query.exec(execWord)) {
            if (1 == query.size()) {        //UPDATE
                execWord = QString::fromLocal8Bit("UPDATE 资产变化 "
                                                  "SET 时间='%1', 变化类别='%2', 资产帐户代号1='%3', 变化资金=%4, "
                                                  "资产帐户代号2='%5', 代号='%6', 名称='%7', 单价=%8, 数量=%9 "
                                                  "WHERE id=%10")
                        .arg(exchangeTime).arg(exchangeType).arg(exchangeData.account1).arg(exchangeData.money)
                        .arg(exchangeData.account2).arg(exchangeData.code).arg(exchangeData.name).arg(exchangeData.price).arg(exchangeData.amount).arg(exchangeData.id);
                qDebug() << execWord;

                if(!query.exec(execWord)) {
                    qDebug() << query.lastError().text();
                    return false;
                } else {
                    for (int i = 0; i < list.count(); i++) {
                        if (data[i].id == exchangeData.id) {
                            QString exchangeStr = updateStrFromExchangeData(exchangeData);
                            data[i] = exchangeData;
                            list.replace(i, exchangeStr);
                        }
                    }
                }
            } else if (0 == query.size()) { //INSERT
                execWord = QString::fromLocal8Bit("INSERT INTO 资产变化 "
                                                  "VALUES (null, '%1', '%2', '%3', %4, '%5', '%6', '%7', %8, %9)")
                        .arg(exchangeTime).arg(exchangeType).arg(exchangeData.account1).arg(exchangeData.money)
                        .arg(exchangeData.account2).arg(exchangeData.code).arg(exchangeData.name).arg(exchangeData.price).arg(exchangeData.amount);
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
    }

    // 2 更新list，刷新
    return initial();
}
QString myExchangeListModel::updateStrFromExchangeData(const myExchangeData &exchangeData) {
    QString exchangeStr;
    if (MY_CASH == exchangeData.code && 1 == exchangeData.amount) {
        if (qAbs(exchangeData.price + exchangeData.money) > 0.0001f)
            qDebug() << "[转帐]" << exchangeData.account1 << " " << exchangeData.money
                     << "!="    << exchangeData.account2 << " " << exchangeData.price;
        QString strMoney = QString::number(exchangeData.money, 'f', 2);
        exchangeStr = QString::fromLocal8Bit("[%1]%2->%3(￥%4)")
                .arg(exchangeData.type)
                .arg(exchangeData.account1).arg(exchangeData.account2).arg(strMoney);
    } else {
        QString strMoney = QString::number(exchangeData.money, 'f', 2);
        QString strPrice = QString::number(exchangeData.price, 'f', 2);
        exchangeStr = QString::fromLocal8Bit("[%1]%2(￥%3) - %4@%5(%6*%7)")
                .arg(exchangeData.type)
                .arg(exchangeData.account1).arg(strMoney)
                .arg(exchangeData.name).arg(exchangeData.account2)
                .arg(exchangeData.amount).arg(strPrice);
    }
    return exchangeStr;
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

            QString exchangeStr = updateStrFromExchangeData(tmpExchange);
            // 下标为i的list与data要保持对应的
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

void myExchangeListModel::coordinatorModifyExchange(const myExchangeData &originData, const myExchangeData &targetData, int &changeIdx) {
    changeIdx = NO_DO_EXCHANGE;

    if (originData.account1 != targetData.account1) {
        changeIdx |= ORIG_ACCOUNT_1;
        changeIdx |= TARG_ACCOUNT_1;
    }
    if (   originData.account2 != targetData.account2
        || originData.code != targetData.code
        || (qAbs(originData.price - targetData.price) > MONEY_EPS)) {   //price会影响avgCost的计算
        changeIdx |= ORIG_ACCOUNT_2;
        changeIdx |= TARG_ACCOUNT_2;
    }
    if (qAbs(originData.money - targetData.money) > MONEY_EPS) {
        changeIdx |= ORIG_ACCOUNT_1;
    }
    if (   originData.name != targetData.name
        || originData.amount != targetData.amount) {
        changeIdx |= ORIG_ACCOUNT_2;
    }
    if (   originData.time != targetData.time
        || originData.type != targetData.type) {
        changeIdx |= OTHER_EXCHANGE;
    }

    if (changeIdx & ORIG_ACCOUNT_2) {
        if (!(changeIdx & TARG_ACCOUNT_2)) {
            if (originData.code == MY_CASH && 1 == originData.amount) {
                if (targetData.code != MY_CASH || 1 != targetData.amount) {
                    changeIdx |= TARG_ACCOUNT_2;
                }
            } else {
                if (targetData.code == MY_CASH && 1 == targetData.amount) {
                    changeIdx |= TARG_ACCOUNT_2;
                }
            }
        }
    }
}
