#include "myExchangeListNode.h"
#include "myFinanceDatabase.h"

#include <QDebug>
#include <QString>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

myExchangeListNode::myExchangeListNode()
{
    if (!myFinanceDatabase::isConnected) {
        myFinanceDatabase::connectDB();
    }
}
myExchangeListNode::myExchangeListNode(const myExchangeListNode &otherNode) {
    if (!myFinanceDatabase::isConnected) {
        myFinanceDatabase::connectDB();
    }

    int exchangeCount = exchangeList.count();
    for (int i = 0; i < exchangeCount; i++) {
        exchangeList.append(otherNode.getExchangeList());
    }
}

myExchangeListNode::~myExchangeListNode() {

}

bool myExchangeListNode::initial() {
    exchangeList.clear();

    QSqlQuery query;
    ///读“资产变化”表
    if(query.exec(STR("select * from 资产变化"))) {
        int i = 0;
        while(query.next()) { // 定位结果到下一条记录
            myExchangeData tmpExchange;
            tmpExchange.id                    = query.value(0).toInt();
            tmpExchange.time                  = QDateTime::fromString(query.value(1).toString(), "yyyy-MM-dd hh:mm:ss");
            tmpExchange.exchangeType          = query.value(2).toString();
            tmpExchange.accountMoney          = query.value(3).toString();
            tmpExchange.money                 = query.value(4).toDouble();
            tmpExchange.assetData.accountCode = query.value(5).toString();
            tmpExchange.assetData.assetCode   = query.value(6).toString();
            tmpExchange.assetData.assetName   = query.value(7).toString();
            tmpExchange.assetData.price       = query.value(8).toDouble();
            tmpExchange.assetData.amount      = query.value(9).toInt();

            updateList(tmpExchange);
            i ++;
        }
        qDebug() << "## Initial Exchange data finished, num of exchange data : " << i << "###";
    } else { // 如果查询失败，用下面的方法得到具体数据库返回的原因
        qDebug() << "Fetch Account Data to MySql error: " << query.lastError().text();
        return false;
    }
    return true;
}
void myExchangeListNode::rollback() {
    exchangeList.clear();
}

bool myExchangeListNode::doExchange(myExchangeData &exchangeData, bool isDelete) {
    qDebug() << "### myExchangeListModel::doExchange ###";
    QSqlQuery query;
    QString execWord;
    if (isDelete) {
        execWord = STR("DELETE FROM 资产变化 WHERE id=%1").arg(exchangeData.id);
        MY_DEBUG_SQL(execWord);
        if(!query.exec(execWord)) {
            MY_DEBUG_ERROR(query.lastError().text());
            return false;
        } else {
            int dataCount = exchangeList.count();
            for (int i = 0; i < dataCount-1; i++) {
                if (exchangeList.at(i).id == exchangeData.id) {
                    exchangeList.removeAt(i);
                    break;
                }
            }
            return true;
        }
    } else {
        QString exchangeTime = exchangeData.time.toString(("yyyy-MM-dd hh:mm:ss"));
        QString exchangeType = exchangeData.exchangeType;
        // 1 update database, "资产变化"表 CHANGE
        execWord = STR("SELECT count(*) FROM 资产变化 WHERE id=%1").arg(exchangeData.id);
        int numRows = myFinanceDatabase::getQueryRows(execWord);
        if (1 == numRows) {        //UPDATE
            execWord = STR("UPDATE 资产变化 "
                    "SET 时间='%1', 变化类别='%2', 资产帐户代号1='%3', 变化资金=%4, "
                    "资产帐户代号2='%5', 代号='%6', 名称='%7', 单价=%8, 数量=%9 "
                    "WHERE id=%10")
                    .arg(exchangeTime).arg(exchangeType).arg(exchangeData.accountMoney).arg(exchangeData.money)
                    .arg(exchangeData.assetData.accountCode).arg(exchangeData.assetData.assetCode).arg(exchangeData.assetData.assetName)
                    .arg(exchangeData.assetData.price).arg(exchangeData.assetData.amount).arg(exchangeData.id);
            MY_DEBUG_SQL(execWord);

            if(!query.exec(execWord)) {
                MY_DEBUG_ERROR(query.lastError().text());
                return false;
            } else {
                for (int i = 0; i < exchangeList.count(); i++) {
                    if (exchangeList.at(i).id == exchangeData.id) {
                        exchangeList.replace(i, exchangeData);
                        break;
                    }
                }
                return true;
            }
        } else if (0 == numRows) { //INSERT
            execWord = STR("INSERT INTO 资产变化 "
                                              "VALUES (null, '%1', '%2', '%3', %4, '%5', '%6', '%7', %8, %9)")
                    .arg(exchangeTime).arg(exchangeType).arg(exchangeData.accountMoney).arg(exchangeData.money)
                    .arg(exchangeData.assetData.accountCode).arg(exchangeData.assetData.assetCode).arg(exchangeData.assetData.assetName)
                    .arg(exchangeData.assetData.price).arg(exchangeData.assetData.amount);
            MY_DEBUG_SQL(execWord);

            if(!query.exec(execWord)) {
                MY_DEBUG_ERROR(query.lastError().text());
                return false;
            } else {
                updateList(exchangeData);
                execWord = STR("SELECT last_insert_rowid()");
                MY_DEBUG_SQL(execWord);
                if(query.exec(execWord)) {
                    query.next();
                    exchangeData.id = query.value(0).toInt();
                    updateList(exchangeData);
                    return true;
                } else {
                    MY_DEBUG_ERROR(query.lastError().text());
                    return false;
                }
            }
        } else {
            return false;
        }
    }
    return false;
}

void myExchangeListNode::updateList(const myExchangeData &exchangeData) {
    exchangeList.append(exchangeData);

    int dataCount = exchangeList.count();
    for (int i = 0; i < dataCount-1; i++) {
        if (exchangeList.at(i).time.toMSecsSinceEpoch() > exchangeList.at(dataCount-1).time.toMSecsSinceEpoch()) {
            exchangeList.swap(i, dataCount-1);
        }
    }
}
