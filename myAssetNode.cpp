﻿#include "myAssetNode.h"

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlError>
#include <QtSql/QSqlTableModel>
#include <QtSql/QSqlRecord>
#include <QtSql/QSqlDatabase>

#include <QVariant>
#include <QDebug>

QSqlDatabase myAssetNode::db;

myAssetNode::myAssetNode() {

}

///
/// \brief myAssetNode::myAssetNode
/// \param type
///     {account, hold, root}
/// \param nodeData
///     if root -> nodeData = nullptr
///
myAssetNode::myAssetNode(nodeType type, QVariant nodeData) {
    this->type = type;
    this->nodeData = nodeData;
}
myAssetNode::~myAssetNode() {
    callback();
}

bool myAssetNode::initial() {
    if (!myAssetNode::connectDB())
        return nullptr;

    //rootNode = new myAssetNode(myAssetNode::nodeRoot, "RootNode");

    if (!this) {
        return false;
    } else if (this->type != myAssetNode::nodeRoot) {
        return false;
    }
    myAssetNode* rootNode = this;
    rootNode->parent = nullptr;

    QSqlQuery query;
    int numRows = 0;
    ///读“资产帐户”表
    if(query.exec(QString::fromLocal8Bit("select * from 资产帐户"))) {
        if(db.driver()->hasFeature(QSqlDriver::QuerySize)){
            numRows = query.size(); // 如果支持结果影响的行数，那么直接记录下来
        } else {
            query.last(); //否则定位到结果最后，qt 文档说，这个方法非常慢
            numRows = query.at() + 1;
        }

        int i = 0;
        while(query.next() && i < numRows) { // 定位结果到下一条记录
            myAssetAccount tmpAccount;
            tmpAccount.code = query.value(0).toString();
            tmpAccount.name = query.value(1).toString();
            tmpAccount.type = query.value(2).toString();
            if (tmpAccount.name == QString::fromLocal8Bit("中国工商银行")) {
                tmpAccount.logo = "gsyh.png";
            } else if (tmpAccount.name == QString::fromLocal8Bit("中国招商银行")) {
                tmpAccount.logo = "zsyh.png";
            } else if (tmpAccount.name == QString::fromLocal8Bit("中国银行")) {
                tmpAccount.logo = "zgyh.png";
            } else if (tmpAccount.name == QString::fromLocal8Bit("华泰证券")) {
                tmpAccount.logo = "htzq.png";
            } else if (tmpAccount.name == QString::fromLocal8Bit("国泰君安")) {
                tmpAccount.logo = "gtja.png";
            } else if (tmpAccount.name == QString::fromLocal8Bit("支付宝")) {
                tmpAccount.logo = "zfb.png";
            } else {
                tmpAccount.logo = "nologo";
            }

            QVariant data;
            data.setValue(tmpAccount);
            myAssetNode *account = new myAssetNode(myAssetNode::nodeAccount, data);
            account->parent = rootNode;

            rootNode->addChild(account);

            i ++;
        }
    } else { // 如果查询失败，用下面的方法得到具体数据库返回的原因
        qDebug() << "Fetch Account Data to MySql error: " << query.lastError().text();
        return nullptr;
    }

    query.finish();
    numRows = 0;
    ///读“资产”表
    if(query.exec(QString::fromLocal8Bit("select * from 资产"))) {
        if(db.driver()->hasFeature(QSqlDriver::QuerySize)){
            numRows = query.size(); // 如果支持结果影响的行数，那么直接记录下来
        } else {
            query.last(); //否则定位到结果最后，qt 文档说，这个方法非常慢
            numRows = query.at() + 1;
        }
        int i = 0;

        while(query.next()) { // 定位结果到下一条记录
            myAssetHold tmpHold;
            tmpHold.assetCode   = query.value(0).toString();
            tmpHold.name        = query.value(1).toString();
            tmpHold.accountCode = query.value(2).toString();
            tmpHold.amount      = query.value(3).toInt();
            tmpHold.price       = query.value(4).toFloat();
            tmpHold.type        = query.value(5).toString();

            QVariant data;
            data.setValue(tmpHold);
            myAssetNode *hold = new myAssetNode(myAssetNode::nodeHolds, data);

            myAssetNode *account = rootNode->getAccountNode(tmpHold.accountCode);
            hold->parent = account;
            account->addChild(hold);

            i++;
        }
    } else { // 如果查询失败，用下面的方法得到具体数据库返回的原因
        qDebug() << "Fetch Asset Data to MySql error: " << query.lastError().text();
        return nullptr;
    }

    query.finish();
    numRows = 0;
    ///读“资产变化”表

    return true;
}
bool myAssetNode::callback() {
    myAssetNode* rootNode = this;
    if (!rootNode) {
        return false;
    } else if (rootNode->type != myAssetNode::nodeRoot) {
        return false;
    }

    int assetAccountCount = rootNode->children.count();
    for (int i = assetAccountCount-1; i >= 0; i--) {
        int assetHoldCount = rootNode->children.at(i)->children.count();
        for (int j = assetHoldCount-1; j >= 0; j--) {
            delete rootNode->children.at(i)->children.at(j);
        }
        rootNode->children.at(i)->children.clear();

        delete rootNode->children.at(i);
    }
    rootNode->children.clear();

    return true;
}

bool myAssetNode::connectDB() {
    if (db.isOpen()) {
        db.close();
    } else {
        db = QSqlDatabase::addDatabase("QMYSQL");
        db.setDatabaseName("myfinance");
        db.setHostName("127.0.0.1");
        db.setUserName("root");
        db.setPassword("");
    }

    if (!db.open()) {
        qDebug() << "Connect to MySql error: " << db.lastError().text();
        return false;
    } else {
        return true;
    }
}

void myAssetNode::addChild(myAssetNode *childNode) {
    this->children.append(childNode);
}

myAssetNode *myAssetNode::getAccountNode(QString accountCode) {
    for ( int i = 0; i != children.size(); ++i ) {
        if ((children.at(i)->nodeData).value<myAssetAccount>().code == accountCode ) {
            return children.at(i);
        }
    }
    return nullptr;
}

bool myAssetNode::doExchange(exchangeData data) {
    QSqlQuery query;
    // 1 改变"资产变化"表
//    if(query.exec(QString::fromLocal8Bit("INSERT 资产变化 SET 数量=%1 WHERE 代号='test8'").arg(data.amount))) {
//        return true;
//    } else {
//        qDebug() << query.lastError().text();
//        return false;
//    }

    // 2 改变"资产"表*2
    // 2.1 MONEY CHANGE
    QString filter   = QString::fromLocal8Bit("资产帐户代号='%1' AND 代号='cash'").arg(data.account1);
    QString execWord = QString::fromLocal8Bit("select 单位成本 from 资产"
                                              " WHERE %1").arg(filter);
    if(query.exec(execWord)) {
        if (1 == query.size()) {
            query.next();
            float moneyOrigin = query.value(0).toDouble();
            float money = moneyOrigin+data.money;
            qDebug() << moneyOrigin << "  " << data.money << "  " << money;
            execWord = QString::fromLocal8Bit("UPDATE 资产 SET 单位成本=%1"
                                              " WHERE %2").arg(money).arg(filter);
            qDebug() << execWord;
            if(!query.exec(execWord)) {
                qDebug() << query.lastError().text();
                return false;
            }
        } else {
            qDebug() << "select money error:" << execWord;
            return false;
        }
    } else {
        qDebug() << query.lastError().text();
        return false;
    }
    query.clear();

    // 2.2 ASSET CHANGE
    filter   = QString::fromLocal8Bit("资产帐户代号='%1' AND 代号='%2'").arg(data.account2).arg(data.code);
    execWord = QString::fromLocal8Bit("select 数量, 单位成本 from 资产"
                                      " WHERE %1").arg(filter);
    qDebug() << execWord;
    if(query.exec(execWord)) {
        if (1 == query.size()) {
            //UPDATE
            query.next();
            int   amountOrigin = query.value(0).toInt();
            float priceOrigin  = query.value(1).toDouble();
            float price  = 0.0f;
            int   amount = 0;
            if (data.code != "cash") {
                amount = amountOrigin + data.amount;
                price  = (data.price*data.amount+priceOrigin*amountOrigin)/amount;
            } else {
                amount = 1;
                price  = priceOrigin + data.price;
            }
            if (amount != 0) {
                execWord = QString::fromLocal8Bit("UPDATE 资产 SET 数量=%1, 单位成本=%2"
                                                  " WHERE %3").arg(amount).arg(price).arg(filter);
            } else {
                execWord = QString::fromLocal8Bit("delete from 资产"
                                                  " WHERE %1").arg(filter);
            }
            qDebug() << execWord;
            if(!query.exec(execWord)) {
                qDebug() << query.lastError().text();
                return false;
            }
        } else if (0 == query.size()) {
            // INSERT
            execWord = QString::fromLocal8Bit("INSERT INTO 资产 "
                                              "VALUES ('%1', '%2', '%3', %4, %5, '%6')")
                    .arg(data.code).arg(data.name).arg(data.account2).arg(data.amount).arg(data.price).arg(data.type);
            qDebug() << execWord;
            if(!query.exec(execWord)) {
                qDebug() << query.lastError().text();
                return false;
            }
        } else {
            qDebug() << "大于一条记录 ERROR:" << execWord;
            return false;
        }
    } else {
        qDebug() << query.lastError().text();
        return false;
    }
    query.clear();
    return true;
}