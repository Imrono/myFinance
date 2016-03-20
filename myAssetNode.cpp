#include "myAssetNode.h"

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlError>
#include <QtSql/QSqlRecord>
#include "myFinanceDatabase.h"

#include <QVariant>
#include <QDebug>

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
    if (!myFinanceDatabase::isConnected) {
        if (!myFinanceDatabase::connectDB())
            return nullptr;
    }

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
        if(myFinanceDatabase::db.driver()->hasFeature(QSqlDriver::QuerySize)){
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
        if(myFinanceDatabase::db.driver()->hasFeature(QSqlDriver::QuerySize)){
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
    // 改变"资产"表*2
    // 1 MONEY CHANGE
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

    // 2 ASSET CHANGE
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
            float avgCost  = 0.0f;
            int   amount = 0;
            if (data.code != "cash") {
                amount = amountOrigin + data.amount;
                avgCost  = (data.price*data.amount + priceOrigin*amountOrigin + data.fee)/amount;
            } else {
                amount = 1;
                avgCost  = priceOrigin + data.price - data.fee;
            }
            if (amount != 0) {
                execWord = QString::fromLocal8Bit("UPDATE 资产 SET 数量=%1, 单位成本=%2"
                                                  " WHERE %3").arg(amount).arg(avgCost).arg(filter);
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

bool myAssetNode::checkExchange(const exchangeData &data, QString &abnormalInfo) {
    exchangeAbnomal abnormalCode = NORMAL;

    if (qAbs(data.money + data.fee) < MONEY_EPS) {
        abnormalCode = MONEY_ZERO;
        abnormalInfo = QString::fromLocal8Bit("%1's No money exchange").arg(data.account1);
        return false;
    } else if (data.price < MONEY_EPS) {
        abnormalCode = PRICE_ZERO;
        abnormalInfo = QString::fromLocal8Bit("%1's exchange price 0.0").arg(data.account2);
        return false;
    } else {}

    QSqlQuery query;
    // check "资产"表*2
    // 1 MONEY CHECK
    QString filter   = QString::fromLocal8Bit("资产帐户代号='%1' AND 代号='cash'").arg(data.account1);
    QString execWord = QString::fromLocal8Bit("select 单位成本 from 资产"
                                              " WHERE %1").arg(filter);
    if(query.exec(execWord)) {
        if (1 == query.size()) {
            query.next();
            float moneyOrigin = query.value(0).toDouble();
            float money = moneyOrigin + data.money;
            qDebug() << moneyOrigin << "  " << data.money << "  " << money;
            if (money < 0.0f) {
                abnormalCode = LACK_MONEY_1;
                abnormalInfo = QString::fromLocal8Bit("%1's 现在资金 %2 需要资金 %3")
                        .arg(data.account1).arg(moneyOrigin).arg(data.money);
                return false;
            }
        } else if (0 == query.size()) {
            abnormalCode = NO_MONEY_ATTRIBUTE;
            abnormalInfo = QString::fromLocal8Bit("数据库中 %1 没有cash属性").arg(data.account1);
            return false;
        } else {
            qDebug() << "select money error:" << execWord;
            abnormalCode = UN_UNIQUE_1;
            abnormalInfo = QString::fromLocal8Bit("数据库中 %1's cash 查找结果不唯一").arg(data.account1);
            return false;
        }
    } else {
        qDebug() << query.lastError().text();
        abnormalCode = SQL_ERROR;
        return false;
    }
    query.clear();

    // 2 ASSET CHECK
    filter   = QString::fromLocal8Bit("资产帐户代号='%1' AND 代号='%2'").arg(data.account2).arg(data.code);
    execWord = QString::fromLocal8Bit("select 数量, 单位成本 from 资产"
                                      " WHERE %1").arg(filter);
    qDebug() << execWord;
    if(query.exec(execWord)) {
        if (1 == query.size()) {
            query.next();
            int   amountOrigin = query.value(0).toInt();
            float priceOrigin  = query.value(1).toDouble();
            if (data.code == "cash") {
                float price = priceOrigin + data.price;
                if (price < 0.0f) {
                    abnormalCode = LACK_MONEY_2;
                    abnormalInfo = QString::fromLocal8Bit("%1's 现在资金 %2 需要资金 %3")
                            .arg(data.account2).arg(priceOrigin).arg(data.price);
                    return false;
                }
            } else {
                int amount = amountOrigin + data.amount;
                if (amount < 0) {
                    abnormalCode = LACK_STOCK;
                    abnormalInfo = QString::fromLocal8Bit("%1's 现在股票(%2) %3 需要股票 %4")
                            .arg(data.account2).arg(data.name).arg(amountOrigin).arg(data.amount);
                    return false;
                }
            }
        } else if (0 == query.size()) {
            if (data.amount < 0) {
                abnormalCode = LACK_STOCK;
                abnormalInfo = QString::fromLocal8Bit("%1's 现在股票(%2) 0 需要股票 %3")
                        .arg(data.account2).arg(data.name).arg(data.amount);
                return false;
            } else {}
        } else {
            abnormalCode = UN_UNIQUE_2;
            abnormalInfo = QString::fromLocal8Bit("数据库中 %1's %2 查找结果不唯一")
                    .arg(data.account2).arg(data.name);
            return false;
        }
    } else {
        qDebug() << query.lastError().text();
        abnormalCode = SQL_ERROR;
        abnormalInfo = QString::fromLocal8Bit("SQL ERROR");
        return false;
    }
    query.clear();
    abnormalCode = NORMAL;
    abnormalInfo = QString::fromLocal8Bit("EXCHANGE CHECK OK");
    return true;
}
