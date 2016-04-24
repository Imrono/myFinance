#include "myAssetNode.h"

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlError>
#include <QtSql/QSqlRecord>
#include "myFinanceDatabase.h"

#include <QVariant>
#include <QDebug>

myAssetHold::myAssetHold() {}
myAssetHold::myAssetHold(myAssetData data) {
    this->accountCode = data.accountCode;
    this->assetCode   = data.assetCode;
    this->name        = data.assetName;
    this->amount      = data.amount;
    this->price       = data.price;
    this->type        = data.type;
}
myAssetAccount::myAssetAccount() {}
myAssetAccount::myAssetAccount(myAccountData data) {
    this->code = data.Code;
    this->name = data.Name;
    this->note = data.Note;
    this->type = data.Type;
}

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
}

void myAssetNode::addChild(myAssetNode *childNode) {
    this->children.append(childNode);
}

bool myAssetNode::doExchange(myExchangeData data, myRootAccountAsset &rootNode) {
    QSqlQuery query;
    QString filter;
    QString execWord;

    // 1 MONEY CHANGE
    filter   = STR("资产帐户代号='%1' AND 代号='cash'").arg(data.account1);
    execWord = STR("select 单位成本 from 资产 WHERE %1").arg(filter);
    if(query.exec(execWord)) {
        if (1 == query.size()) {
            query.next();
            float moneyOrigin = query.value(0).toDouble();
            float money = moneyOrigin + data.money;
            QString strMoney = QString::number(money, 'f', 3);
            qDebug() << moneyOrigin << "  " << data.money << "  " << money;
            execWord = STR("UPDATE 资产 SET 单位成本=%1 WHERE %2").arg(strMoney).arg(filter);
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
    filter   = STR("资产帐户代号='%1' AND 代号='%2'").arg(data.account2).arg(data.code);
    execWord = STR("select 数量, 单位成本 from 资产 WHERE %1").arg(filter);
    qDebug() << execWord;
    if(query.exec(execWord)) {
        int   amountOrigin = 0;
        float priceOrigin  = 0.0f;
        float avgCost  = 0.0f;
        int   amount = 0;

        if (1 == query.size()) {
            //UPDATE
            query.next();
            amountOrigin = query.value(0).toInt();
            priceOrigin  = query.value(1).toDouble();
            if (data.code != "cash") {
                amount = amountOrigin + data.amount;
                avgCost = (data.price*data.amount + priceOrigin*amountOrigin + data.fee)/amount;
            } else {
                amount = 1;
                avgCost = priceOrigin + data.price - data.fee;
            }
            if (amount != 0) {  //即使为0，"cash"不会被删除
                execWord = STR("UPDATE 资产 SET 数量=%1, 单位成本=%2, 名称=%3 WHERE %5")
                                    .arg(amount).arg(avgCost).arg(data.name).arg(filter);
            } else {
                execWord = STR("delete from 资产 WHERE %1").arg(filter);
            }
            qDebug() << execWord;
            if(!query.exec(execWord)) {
                qDebug() << query.lastError().text();
                return false;
            }
        } else if (0 == query.size()) {
            // INSERT
            if (data.code != "cash") {
                amount = amountOrigin + data.amount;
                avgCost = (data.price*data.amount + priceOrigin*amountOrigin + data.fee)/amount;
            } else {
                amount = 1;
                avgCost = priceOrigin + data.price - data.fee;
            }
            execWord = STR("INSERT INTO 资产 VALUES ('%1', '%2', '%3', %4, %5, '%6', %7)")
                    .arg(data.code).arg(data.name).arg(data.account2).arg(amount).arg(avgCost)
                    .arg(data.type).arg(rootNode.getAccountNode(data.account2)->children.count());
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
bool myAssetNode::checkExchange(const myExchangeData &data, QString &abnormalInfo) {
    exchangeAbnomal abnormalCode = NORMAL;

    if (qAbs(data.money + data.fee) < MONEY_EPS) {
        abnormalCode = MONEY_ZERO;
        abnormalInfo = STR("%1's No money exchange").arg(data.account1);
        return false;
    } else if (data.price < MONEY_EPS) {
        abnormalCode = PRICE_ZERO;
        abnormalInfo = STR("%1's exchange price 0.0").arg(data.account2);
        return false;
    } else {}

    QSqlQuery query;
    // check "资产"表*2
    // 1 MONEY CHECK
    QString filter   = STR("资产帐户代号='%1' AND 代号='cash'").arg(data.account1);
    QString execWord = STR("select 单位成本 from 资产"
                                              " WHERE %1").arg(filter);
    if(query.exec(execWord)) {
        if (1 == query.size()) {
            query.next();
            float moneyOrigin = query.value(0).toDouble();
            float money = moneyOrigin + data.money;
            qDebug() << moneyOrigin << "  " << data.money << "  " << money;
            if (money < 0.0f) {
                abnormalCode = LACK_MONEY_1;
                abnormalInfo = STR("%1's 现在资金 %2 需要资金 %3")
                        .arg(data.account1).arg(moneyOrigin).arg(data.money);
                return false;
            }
        } else if (0 == query.size()) {
            abnormalCode = NO_MONEY_ATTRIBUTE;
            abnormalInfo = STR("数据库中 %1 没有cash属性").arg(data.account1);
            return false;
        } else {
            qDebug() << "select money error:" << execWord;
            abnormalCode = UN_UNIQUE_1;
            abnormalInfo = STR("数据库中 %1's cash 查找结果不唯一").arg(data.account1);
            return false;
        }
    } else {
        qDebug() << query.lastError().text();
        abnormalCode = SQL_ERROR;
        return false;
    }
    query.clear();

    // 2 ASSET CHECK
    filter   = STR("资产帐户代号='%1' AND 代号='%2'").arg(data.account2).arg(data.code);
    execWord = STR("select 数量, 单位成本 from 资产"
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
                    abnormalInfo = STR("%1's 现在资金 %2 需要资金 %3")
                            .arg(data.account2).arg(priceOrigin).arg(data.price);
                    return false;
                }
            } else {
                int amount = amountOrigin + data.amount;
                if (amount < 0) {
                    abnormalCode = LACK_STOCK;
                    abnormalInfo = STR("%1's 现在股票(%2) %3 需要股票 %4")
                            .arg(data.account2).arg(data.name).arg(amountOrigin).arg(data.amount);
                    return false;
                }
            }
        } else if (0 == query.size()) {
            if (data.amount < 0) {
                abnormalCode = LACK_STOCK;
                abnormalInfo = STR("%1's 现在股票(%2) 0 需要股票 %3")
                        .arg(data.account2).arg(data.name).arg(data.amount);
                return false;
            } else {}
        } else {
            abnormalCode = UN_UNIQUE_2;
            abnormalInfo = STR("数据库中 %1's %2 查找结果不唯一")
                    .arg(data.account2).arg(data.name);
            return false;
        }
    } else {
        qDebug() << query.lastError().text();
        abnormalCode = SQL_ERROR;
        abnormalInfo = STR("SQL ERROR");
        return false;
    }
    query.clear();
    abnormalCode = NORMAL;
    abnormalInfo = STR("EXCHANGE CHECK OK");
    Q_UNUSED(abnormalCode);
    return true;
}


myRootAccountAsset::myRootAccountAsset() : rootNode(myAssetNode::nodeRoot, "RootNode")
{
    rootNode.parent = nullptr;
}
myRootAccountAsset::~myRootAccountAsset() {
    callback();
}

bool myRootAccountAsset::initial() {
    if (!myFinanceDatabase::isConnected) {
        if (!myFinanceDatabase::connectDB())
            return false;
    }

    QSqlQuery query;
    int numRows = 0;
    ///读“资产帐户”表
    if(query.exec(STR("select * from 资产帐户"))) {
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
            tmpAccount.note = query.value(3).toString();
            tmpAccount.pos  = query.value(4).toInt();
            if (tmpAccount.name.contains(STR("工商银行"))) {
                tmpAccount.logo = "gsyh.png";
            } else if (tmpAccount.name.contains(STR("招商银行"))) {
                tmpAccount.logo = "zsyh.png";
            } else if (tmpAccount.name == STR("中国银行")) {
                tmpAccount.logo = "zgyh.png";
            } else if (tmpAccount.name == STR("华泰证券")) {
                tmpAccount.logo = "htzq.png";
            } else if (tmpAccount.name == STR("国泰君安")) {
                tmpAccount.logo = "gtja.png";
            } else if (tmpAccount.name == STR("支付宝")) {
                tmpAccount.logo = "zfb.png";
            } else {
                tmpAccount.logo = "nologo";
            }

            QVariant data;
            data.setValue(tmpAccount);
            myAssetNode *account = new myAssetNode(myAssetNode::nodeAccount, data);
            account->parent = &rootNode;

            if (tmpAccount.code != OTHER_ACCOUNT)
                rootNode.addChild(account);

            i ++;
        }
    } else { // 如果查询失败，用下面的方法得到具体数据库返回的原因
        qDebug() << "Fetch Account Data to MySql error: " << query.lastError().text();
        return false;
    }

    query.finish();
    numRows = 0;
    ///读“资产”表
    if(query.exec(STR("select * from 资产"))) {
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
            tmpHold.pos         = query.value(6).toInt();

            QVariant data;
            data.setValue(tmpHold);
            myAssetNode *hold = new myAssetNode(myAssetNode::nodeHolds, data);

            myAssetNode *account = getAccountNode(tmpHold.accountCode);
            hold->parent = account;
            account->addChild(hold);

            i++;
            qDebug() << "account:" << tmpHold.accountCode << " asset:" << tmpHold.assetCode;
        }
    } else { // 如果查询失败，用下面的方法得到具体数据库返回的原因
        qDebug() << "Fetch Asset Data to MySql error: " << query.lastError().text();
        return false;
    }

    query.finish();
    numRows = 0;
    ///读“资产变化”表

    doSortPosition();

    return true;
}
bool myRootAccountAsset::callback() {
    int assetAccountCount = rootNode.children.count();
    for (int i = assetAccountCount-1; i >= 0; i--) {
        int assetHoldCount = rootNode.children.at(i)->children.count();
        for (int j = assetHoldCount-1; j >= 0; j--) {
            delete rootNode.children.at(i)->children.at(j);
        }
        rootNode.children.at(i)->children.clear();

        delete rootNode.children.at(i);
    }
    rootNode.children.clear();

    return true;
}

myAssetNode *myRootAccountAsset::getAccountNode(QString accountCode) const {
    for ( int i = 0; i != rootNode.children.size(); ++i ) {
        if ((rootNode.children.at(i)->nodeData).value<myAssetAccount>().code == accountCode ) {
            return rootNode.children.at(i);
        }
    }
    return nullptr;
}
myAssetNode *myRootAccountAsset::getAccountNode(int i) const {
    int count = rootNode.children.size();
    if (i < 0 || i >= count) {
        return nullptr;
    } else {
        return rootNode.children.at(i);
    }
}

QStringList myRootAccountAsset::getAllStockCodeList() {
    QStringList list;
    int numAccount = rootNode.children.size();
    for (int i = 0; i < numAccount; i++) {
        myAssetNode *account = rootNode.children.at(i);
        int numAsset = account->children.size();
        for (int j = 0; j < numAsset; j++) {
            myAssetNode *asset = account->children.at(j);
            list.append((asset->nodeData).value<myAssetHold>().assetCode);
        }
    }
    list.removeDuplicates();

    for (QStringList::iterator it = list.begin(); it != list.end();) {
        qDebug() << *it;
        if (!it->contains(".")) {
            it = list.erase(it);
        } else {
            ++it;
        }
    }
    return list;
}

bool myRootAccountAsset::doChangeAssetDirectly(const myAssetNode *node, changeType type, QVariant data) {
    QSqlQuery query;
    QString execWord, filter;

    if (myAssetNode::nodeAccount == node->type) {
        /// INSERT ASSET
        if (POP_INSERT == type) {
            myAssetData assetData = data.value<myAssetData>();

            filter   = STR("资产帐户代号='%1' AND 代号='%2'")
                            .arg(assetData.accountCode).arg(assetData.assetCode);
            execWord = STR("select * from 资产 WHERE %1").arg(filter);
            qDebug() << execWord;
            if(query.exec(execWord)) {
                if (0 == query.size()) {
                    myAssetNode *tmpAccount = getAccountNode(assetData.accountCode);
                    QString strPrice = QString::number(assetData.price, 'f', 3);
                    execWord = STR("INSERT INTO 资产 VALUES ('%1', '%2', '%3', %4, %5, '%6', %7)")
                            .arg(assetData.assetCode).arg(assetData.assetName).arg(assetData.accountCode)
                            .arg(assetData.amount).arg(strPrice).arg(assetData.type).arg(tmpAccount->children.count());
                    qDebug() << execWord;
                    if(query.exec(execWord)) {
                        myAssetHold tmpHold(assetData);
                        QVariant data;
                        data.setValue(tmpHold);
                        myAssetNode *hold = new myAssetNode(myAssetNode::nodeHolds, data);

                        hold->parent = tmpAccount;
                        tmpAccount->addChild(hold);

                        return true;
                    } else {
                        qDebug() << query.lastError().text();
                        return false;}
                } else { return false;}
            } else { return false;}
        /// MODIFY ACCOUNT
        } else if (POP_MODIFY == type) {
            myAccountData accountData = data.value<myAccountData>();

            filter   = STR("代号='%1'").arg(accountData.originCode);
            execWord = STR("select * from 资产帐户 WHERE %1").arg(filter);
            qDebug() << execWord;
            if(query.exec(execWord)) {
                if (1 == query.size()) {
                    execWord = STR("UPDATE 资产帐户 SET 代号='%1', 名称='%2', 类别='%3', 备注='%4' WHERE %5")
                            .arg(accountData.Code).arg(accountData.Name).arg(accountData.Type).arg(accountData.Note)
                            .arg(filter);
                    qDebug() << execWord;
                    if(!query.exec(execWord)) {
                        qDebug() << query.lastError().text();
                        return false;
                    } else { return true;}
                } else { return false;}
            } else { return false;}
        /// DELETE ACCOUNT
        } else if (POP_DELETE == type) {
            QString accountCode = node->nodeData.value<myAssetAccount>().code;
            // delete holds
            int count = node->children.count();
            for (int i = count-1; i >= 0; i--) {
                QString assetCode = node->children.at(i)->nodeData.value<myAssetHold>().assetCode;
                qDebug() << "delete " << i << "@total:" << node->children.count();
                if (!deleteOneAsset(accountCode, assetCode)) {
                    return false;
                }
            }
            // delete account
            filter   = STR("代号='%1'").arg(accountCode);
            execWord = STR("select * from 资产帐户 WHERE %1").arg(filter);
            qDebug() << execWord;
            if(query.exec(execWord)) {
                if (1 == query.size()) {
                    query.next();
                    int pos  = query.value(4).toInt();

                    execWord = STR("delete from 资产帐户 WHERE %1").arg(filter);
                    qDebug() << execWord;
                    if(query.exec(execWord)) {
                        int toRemove = -1;
                        for (int i = 0; i < rootNode.children.count(); i++) {
                            const myAssetAccount &tmpAccount = rootNode.children.at(i)->nodeData.value<myAssetAccount>();
                            if (tmpAccount.code == accountCode) {
                                toRemove = i;
                            } else if (tmpAccount.pos > pos) {
                                if (!setAccountPosition(tmpAccount.code, tmpAccount.pos-1)) {
                                        return false;
                                }
                            }
                        }
                        rootNode.children.removeAt(toRemove);
                        return true;
                    } else { return false;}
                } else { return false;}
            } else { return false;}
        } else { return false;}
    } else if (myAssetNode::nodeHolds == node->type) {
        /// MODIFY ASSET
        if (POP_MODIFY == type) {
            myAssetData assetData = data.value<myAssetData>();

            filter   = STR("资产帐户代号='%1' AND 代号='%2'")
                            .arg(assetData.originAccountCode).arg(assetData.originAssetCode);
            execWord = STR("select * from 资产 WHERE %1").arg(filter);
            qDebug() << execWord;
            if(query.exec(execWord)) {
                if (1 == query.size()) {
                    QString strPrice = QString::number(assetData.price, 'f', 3);
                    execWord = STR("UPDATE 资产 SET 代号='%1', 名称='%2', 资产帐户代号='%3', 数量=%4, 单位成本=%5, 类别='%6' "
                                                      "WHERE %7")
                            .arg(assetData.assetCode).arg(assetData.assetName).arg(assetData.accountCode)
                            .arg(assetData.amount).arg(strPrice).arg(assetData.type)
                            .arg(filter);
                    qDebug() << execWord;
                    if(!query.exec(execWord)) {
                        qDebug() << query.lastError().text();
                        return false;
                    } else { return true;}
                } else { return false;}
            } else { return false;}
        /// DELETE ASSET
        } else if (POP_DELETE == type) {
            // delete holds
            QString accountCode = node->nodeData.value<myAssetHold>().accountCode;
            QString assetCode = node->nodeData.value<myAssetHold>().assetCode;
            return deleteOneAsset(accountCode, assetCode);
        }
    } else { return false;}
    return false;
}
bool myRootAccountAsset::deleteOneAsset(const QString &accountCode, const QString &assetCode) {
    QSqlQuery query;
    QString filter   = STR("资产帐户代号='%1' AND 代号='%2'").arg(accountCode).arg(assetCode);
    QString execWord = STR("select * from 资产 WHERE %1").arg(filter);
    qDebug() << execWord;
    if(query.exec(execWord)) {
        if (1 == query.size()) {
            query.next();
            int pos = query.value(6).toInt();

            execWord = STR("delete from 资产 WHERE %1").arg(filter);
            qDebug() << execWord;
            if(query.exec(execWord)) {
                myAssetNode *tmpAccount = getAccountNode(accountCode);
                int toRemove = -1;
                for (int i = 0; i < tmpAccount->children.count(); i++) {
                    const myAssetHold &tmpHold = tmpAccount->children.at(i)->nodeData.value<myAssetHold>();
                    if (tmpHold.assetCode == assetCode) {
                        toRemove = i;
                    } else if (tmpHold.pos > pos) {
                        if (!setAssetPosition(accountCode, tmpHold.assetCode, tmpHold.pos-1)) {
                            return false;
                        }
                    }
                }
                tmpAccount->children.removeAt(toRemove);
                return true;
            } else { return false;}
        } else { return false;}
    } else { return false;}
}
bool myRootAccountAsset::doInsertAccount(myAccountData data) {
    QSqlQuery query;
    QString execWord, filter;
    filter   = STR("代号='%1'").arg(data.Code);
    execWord = STR("select * from 资产帐户 WHERE %1").arg(filter);
    qDebug() << execWord;
    if(query.exec(execWord)) {
        if (0 == query.size()) {
            execWord = STR("INSERT INTO 资产帐户 "
                                              "VALUES ('%1', '%2', '%3', '%4', %5)")
                    .arg(data.Code).arg(data.Name).arg(data.Type).arg(data.Note).arg(rootNode.children.count());
            qDebug() << execWord;
            if(query.exec(execWord)) {
                myAssetAccount tmpAccount(data);
                QVariant data;
                data.setValue(tmpAccount);
                myAssetNode *account = new myAssetNode(myAssetNode::nodeAccount, data);
                account->parent = &rootNode;

                rootNode.addChild(account);
                return true;
            } else {
                qDebug() << query.lastError().text();
                return false;}
        } else { return false;}
    } else { return false;}
    return false;
}

/// POSITION
bool myRootAccountAsset::setAccountPosition(const QString &accountCode, int pos) {
    QSqlQuery query;
    QString filter   = STR("代号='%1'").arg(accountCode);
    QString execWord = STR("select * from 资产帐户 WHERE %1").arg(filter);
    qDebug() << execWord;
    if(query.exec(execWord)) {
        if (1 == query.size()) {
            execWord = STR("UPDATE 资产帐户 SET pos='%1' WHERE %2")
                    .arg(pos).arg(filter);
            qDebug() << execWord;
            if(query.exec(execWord)) {
                for (int i = 0; i < rootNode.children.count(); i++) {
                    myAssetAccount tmpAccount = rootNode.children.at(i)->nodeData.value<myAssetAccount>();
                    if (tmpAccount.name == accountCode) {
                        tmpAccount.pos = pos;
                        break;
                    }
                }
                return true;
            } else {
                qDebug() << query.lastError().text();
                return false;}
        } else { return false;}
    } else { return false;}
}
bool myRootAccountAsset::setAssetPosition(const QString &accountCode, const QString &assetCode, int pos) {
    QSqlQuery query;
    QString filter   = STR("资产帐户代号='%1' AND 代号='%2'")
                    .arg(accountCode).arg(assetCode);
    QString execWord = STR("select * from 资产 WHERE %1").arg(filter);
    qDebug() << execWord;
    if(query.exec(execWord)) {
        if (1 == query.size()) {
            execWord = STR("UPDATE 资产 SET pos='%1' WHERE %2")
                    .arg(pos).arg(filter);
            qDebug() << execWord;
            if(query.exec(execWord)) {
                myAssetNode *tmpAccount = getAccountNode(accountCode);
                for (int i = 0; i < tmpAccount->children.count(); i++) {
                    myAssetHold tmpHold = tmpAccount->children.at(i)->nodeData.value<myAssetHold>();
                    if (tmpHold.name == assetCode) {
                        tmpHold.pos = pos;
                        tmpAccount->children.at(i)->nodeData.setValue(tmpHold);
                        break;
                    }
                }
                return true;
            } else {
                qDebug() << query.lastError().text();
                return false;;}
        } else { return false;}
    } else { return false;}
}

void myRootAccountAsset::doSortPosition() {
    /// SORT ACCOUNT
    sortPositionAccount();

    /// SORT ASSET
    int numOfAccount = rootNode.children.count();
    for (int i = 0; i < numOfAccount; i++) {
        sortPositionAsset(rootNode.children.at(i));
    }
}
void myRootAccountAsset::sortPositionAccount() {
    QList<int> left;
    QList<int> posList;
    // 0 ~ numOfAsset-1 中找一个没用到的，给重复使用或超出范围的使用
    int numOfAccount = rootNode.children.count();
    for (int i = 0; i < numOfAccount; i++) {
        left.append(i);
    }
    for (int i = 0; i < numOfAccount; i++) {
        const myAssetAccount &tmpHold = (rootNode.children.at(i)->nodeData).value<myAssetAccount>();
        posList.append(tmpHold.pos);
        for (int j = 0; j < left.count(); j++) {
            if (tmpHold.pos == left.at(j)) {
                left.removeAt(j);
                break;
            }
        }
    }

    for (int i = 0; i < numOfAccount; i++) {
        myAssetAccount tmpAccount = (rootNode.children.at(i)->nodeData).value<myAssetAccount>();
        if (tmpAccount.pos < 0 || tmpAccount.pos > numOfAccount - 1 || posList.count(tmpAccount.pos)>1 ) {
            int tmpPos = tmpAccount.pos;
            tmpAccount.pos = left.back();
            if (setAccountPosition(tmpAccount.code, tmpAccount.pos)) {
                for (int j = 0; j < posList.count(); j++) {
                    if (posList.at(j) == tmpPos) {
                        posList.removeAt(j);
                    }
                }
                posList.append(tmpAccount.pos);
                left.pop_back();
            } else {} //sql error
        } else {}
    }

    // 按排好的顺序更新rootNode的children
    QList<myAssetNode *> tmpChild;
    for (int i = 0; i < numOfAccount; i++) {
        for (int j = 0; j < numOfAccount; j++) {
            if (i == (rootNode.children.at(j)->nodeData).value<myAssetAccount>().pos) {
                tmpChild.append(rootNode.children.at(j));
                break;
            }
        }
    }
    rootNode.children = tmpChild;
}
void myRootAccountAsset::sortPositionAsset(myAssetNode *accountNode) {
    if (myAssetNode::nodeAccount != accountNode->type) {
        return;
    }

    QList<int> left;
    QList<int> posList;
    // 0 ~ numOfAsset-1 中找一个没用到的，给重复使用或超出范围的使用
    int numOfAsset = accountNode->children.count();
    for (int i = 0; i < numOfAsset; i++) {
        left.append(i);
    }
    for (int i = 0; i < numOfAsset; i++) {
        const myAssetHold &tmpHold = (accountNode->children.at(i)->nodeData).value<myAssetHold>();
        posList.append(tmpHold.pos);
        for (int j = 0; j < left.count(); j++) {
            if (tmpHold.pos == left.at(j)) {
                left.removeAt(j);
                break;
            }
        }
    }

    for (int i = 0; i < numOfAsset; i++) {
        myAssetHold tmpHold = (accountNode->children.at(i)->nodeData).value<myAssetHold>();
        if (tmpHold.pos < 0 || tmpHold.pos > numOfAsset - 1 || posList.count(tmpHold.pos)>1 ) {
            int tmpPos = tmpHold.pos;
            tmpHold.pos = left.back();
            if (setAssetPosition(tmpHold.accountCode, tmpHold.assetCode, tmpHold.pos)) {
                for (int j = 0; j < posList.count(); j++) {
                    if (posList.at(j) == tmpPos) {
                        posList.removeAt(j);
                    }
                }
                posList.append(tmpHold.pos);
                left.pop_back();
                (accountNode->children.at(i)->nodeData).setValue(tmpHold);
            } else {} //sql error
        } else {}
    }

    // 按排好的顺序更新accountNode的children
    numOfAsset = accountNode->children.count();
    QList<myAssetNode *> tmpChild;
    for (int i = 0; i < numOfAsset; i++) {
        for (int j = 0; j < numOfAsset; j++) {
            myAssetHold tmpHold = (accountNode->children.at(j)->nodeData).value<myAssetHold>();
            if (i == tmpHold.pos) {
                tmpChild.append(accountNode->children.at(j));
                break;
            }
        }
    }
    accountNode->children = tmpChild;
}
