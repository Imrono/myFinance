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
    assetData = data;
}
myAssetAccount::myAssetAccount() {}
myAssetAccount::myAssetAccount(myAccountData data) {
    this->accountData = data;
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
myAssetNode::myAssetNode(nodeType type, const QVariant &nodeData) {
    this->type = type;
    this->nodeData = nodeData;
}
myAssetNode::~myAssetNode() {
}

void myAssetNode::addChild(myAssetNode *childNode) {
    this->children.append(childNode);
}
myAssetNode *myAssetNode::getAssetNode(const QString &assetCode) {
    if (myAssetNode::nodeAccount == type) {
        for ( int i = 0; i != children.size(); ++i ) {
            if ((children.at(i)->nodeData).value<myAssetHold>().assetData.assetCode == assetCode) {
                return children.at(i);
            }
        }
        return nullptr;
    } else {
        return nullptr;
    }
}

bool myAccountAssetRootNode::doExchange(const myAssetData &assetData) {
    QSqlQuery query;
    QString filter;
    QString execWord;

    filter   = STR("资产帐户代号='%1' AND 代号='%2'").arg(assetData.accountCode).arg(assetData.assetCode);
    execWord = STR("select count(*) from 资产 WHERE %1").arg(filter);
    int numRows = myFinanceDatabase::getQueryRows(execWord);
    if (1 == numRows) {
        if (assetData.amount != 0) {      ///UPDATE
            execWord = STR("UPDATE 资产 SET 数量=%1, 单位成本=%2 WHERE %3")
                                .arg(assetData.amount).arg(assetData.price).arg(filter);
        } else {                ///DELETE
            execWord = STR("delete from 资产 WHERE %1").arg(filter);
        }
        qDebug() << execWord;
        if(query.exec(execWord)) {
            /// UPDATE MEMORY DATA
            myAssetNode *account = getAccountNode(assetData.accountCode);
            if (account) {
                myAssetNode *asset = account->getAssetNode(assetData.assetCode);
                if (asset) {    /// update MY_CASH
                    myAssetHold holds = asset->nodeData.value<myAssetHold>();
                    holds.assetData.amount = assetData.amount;
                    holds.assetData.price  = assetData.price;
                    asset->nodeData.setValue(holds);
                    return true;
                }
            }
        } else {
            qDebug() << query.lastError().text() << " numRows=" << numRows;
            return false;
        }
    } else if (0 == numRows) {  ///INSERT
        myAssetNode *account = getAccountNode(assetData.accountCode);
        execWord = STR("INSERT INTO 资产 VALUES ('%1', '%2', '%3', %4, %5, '%6', %7)")
                .arg(assetData.assetCode).arg(assetData.assetName).arg(assetData.accountCode).arg(assetData.amount)
                .arg(assetData.price).arg(assetData.type).arg(account->children.count());
        qDebug() << execWord;
        if(query.exec(execWord)) {
            /// INSERT MEMORY DATA
            myAssetHold holds(assetData);
            holds.pos = getAccountNode(assetData.accountCode)->children.count();
            QVariant data;
            data.setValue(holds);
            myAssetNode *asset = new myAssetNode(myAssetNode::nodeHolds, data);
            account->addChild(asset);
            return true;
        } else {
            qDebug() << query.lastError().text() << " numRows=" << numRows;
            return false;
        }
    } else {
        qDebug() << "大于一条记录 ERROR:" << execWord;
        return false;
    }
    return false;
}

bool myAssetNode::doExchange(myExchangeData data, myAccountAssetRootNode &rootNode) {
    QSqlQuery query;
    QString filter;
    QString execWord;

    // 1 MONEY CHANGE
    filter   = STR("资产帐户代号='%1' AND 代号='cash'").arg(data.accountMoney);
    execWord = STR("select 单位成本 from 资产 WHERE %1").arg(filter);
    qDebug() << execWord;
    if(query.exec(execWord)) {
        execWord = STR("select count(*) from 资产 WHERE %1").arg(filter);
        if (1 == myFinanceDatabase::getQueryRows(execWord)) {
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
    filter   = STR("资产帐户代号='%1' AND 代号='%2'").arg(data.assetData.accountCode).arg(data.assetData.assetCode);
    execWord = STR("select 数量, 单位成本 from 资产 WHERE %1").arg(filter);
    qDebug() << execWord;
    if(query.exec(execWord)) {
        int   amountOrigin = 0;
        float priceOrigin  = 0.0f;
        float avgCost  = 0.0f;
        int   amount = 0;

        execWord = STR("select count(*) from 资产 WHERE %1").arg(filter);
        int numRows = myFinanceDatabase::getQueryRows(execWord);
        if (1 == numRows) {
            //UPDATE
            query.next();
            amountOrigin = query.value(0).toInt();
            priceOrigin  = query.value(1).toDouble();
            if (data.assetData.assetCode != "cash") {
                amount = amountOrigin + data.assetData.amount;
                avgCost = (data.assetData.price*data.assetData.amount + priceOrigin*amountOrigin + data.fee)/amount;
            } else {
                amount = 1;
                avgCost = priceOrigin + data.assetData.price - data.fee;
            }
            if (amount != 0) {  //即使为0，"cash"不会被删除
                execWord = STR("UPDATE 资产 SET 数量=%1, 单位成本=%2 WHERE %3")
                                    .arg(amount).arg(avgCost).arg(filter);
            } else {
                execWord = STR("delete from 资产 WHERE %1").arg(filter);
            }
            qDebug() << execWord;
            if(!query.exec(execWord)) {
                qDebug() << query.lastError().text();
                return false;
            }
        } else if (0 == numRows) {
            // INSERT
            if (data.assetData.assetCode != "cash") {
                amount = amountOrigin + data.assetData.amount;
                avgCost = (data.assetData.price*data.assetData.amount + priceOrigin*amountOrigin + data.fee)/amount;
            } else {
                amount = 1;
                avgCost = priceOrigin + data.assetData.price - data.fee;
            }
            execWord = STR("INSERT INTO 资产 VALUES ('%1', '%2', '%3', %4, %5, '%6', %7)")
                    .arg(data.assetData.assetCode).arg(data.assetData.assetName).arg(data.assetData.accountCode).arg(amount).arg(avgCost)
                    .arg(data.assetData.type).arg(rootNode.getAccountNode(data.assetData.accountCode)->children.count());
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
    qDebug() << "### myAssetNode::checkExchange ###";
    exchangeAbnomal abnormalCode = NORMAL;

    if (qAbs(data.money + data.fee) < MONEY_EPS) {
        abnormalCode = MONEY_ZERO;
        abnormalInfo = STR("%1's No money exchange").arg(data.accountMoney);
        return false;
    } else if (data.assetData.price < MONEY_EPS) {
        abnormalCode = PRICE_ZERO;
        abnormalInfo = STR("%1's exchange price 0.0").arg(data.assetData.accountCode);
        return false;
    } else {}

    QSqlQuery query;
    // check "资产"表*2
    // 1 MONEY CHECK
    QString filter   = STR("资产帐户代号='%1' AND 代号='cash'").arg(data.accountMoney);
    QString execWord = STR("select 单位成本 from 资产 WHERE %1").arg(filter);
    if(query.exec(execWord)) {
        execWord = STR("select count(*) from 资产 WHERE %1").arg(filter);
        int numRows = myFinanceDatabase::getQueryRows(execWord);
        if (1 == numRows) {
            query.next();
            float moneyOrigin = query.value(0).toDouble();
            float money = moneyOrigin + data.money;
            qDebug() << moneyOrigin << "  " << data.money << "  " << money;
            if (money < 0.0f) {
                abnormalCode = LACK_MONEY_1;
                abnormalInfo = STR("%1's 现在资金 %2 需要资金 %3")
                        .arg(data.accountMoney).arg(moneyOrigin).arg(data.money);
                return false;
            }
        } else if (0 == numRows) {
            abnormalCode = NO_MONEY_ATTRIBUTE;
            abnormalInfo = STR("数据库中 %1 没有cash属性").arg(data.accountMoney);
            return false;
        } else {
            qDebug() << "select money error:" << execWord;
            abnormalCode = UN_UNIQUE_1;
            abnormalInfo = STR("数据库中 %1's cash 查找结果不唯一").arg(data.accountMoney);
            return false;
        }
    } else {
        qDebug() << query.lastError().text();
        abnormalCode = SQL_ERROR;
        return false;
    }
    query.clear();

    // 2 ASSET CHECK
    filter   = STR("资产帐户代号='%1' AND 代号='%2'").arg(data.assetData.accountCode).arg(data.assetData.assetCode);
    execWord = STR("select 数量, 单位成本 from 资产 WHERE %1").arg(filter);
    qDebug() << execWord;
    if(query.exec(execWord)) {
        execWord = STR("select count(*) from 资产 WHERE %1").arg(filter);
        int numRows = myFinanceDatabase::getQueryRows(execWord);
        if (1 == numRows) {
            query.next();
            int   amountOrigin = query.value(0).toInt();
            float priceOrigin  = query.value(1).toDouble();
            if (data.assetData.assetCode == "cash") {
                float price = priceOrigin + data.assetData.price;
                if (price < 0.0f) {
                    abnormalCode = LACK_MONEY_2;
                    abnormalInfo = STR("%1's 现在资金 %2 需要资金 %3")
                            .arg(data.assetData.accountCode).arg(priceOrigin).arg(data.assetData.price);
                    return false;
                }
            } else {
                int amount = amountOrigin + data.assetData.amount;
                if (amount < 0) {
                    abnormalCode = LACK_STOCK;
                    abnormalInfo = STR("%1's 现在股票(%2) %3 需要股票 %4")
                            .arg(data.assetData.accountCode).arg(data.assetData.assetName).arg(amountOrigin).arg(data.assetData.amount);
                    return false;
                }
            }
        } else if (0 == numRows) {
            if (data.assetData.amount < 0) {
                abnormalCode = LACK_STOCK;
                abnormalInfo = STR("%1's 现在股票(%2) 0 需要股票 %3")
                        .arg(data.assetData.accountCode).arg(data.assetData.assetName).arg(data.assetData.amount);
                return false;
            } else {}
        } else {
            abnormalCode = UN_UNIQUE_2;
            abnormalInfo = STR("数据库中 %1's %2 查找结果不唯一")
                    .arg(data.assetData.accountCode).arg(data.assetData.assetName);
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


myAccountAssetRootNode::myAccountAssetRootNode() : rootNode(myAssetNode::nodeRoot, "RootNode")
{
    rootNode.parent = nullptr;
}
myAccountAssetRootNode::~myAccountAssetRootNode() {
    callback();
}

bool myAccountAssetRootNode::initial(bool isFetchAccount, bool isFetchAsset) {
    qDebug() << "### myAccountAssetRootNode::initial ###";
    if (!myFinanceDatabase::isConnected) {
        if (!myFinanceDatabase::connectDB())
            return false;
    }

    ///读“资产帐户”表
    if (isFetchAccount) {
        if (fetchAccount()) {
            doSortPosition(true, false);
        } else {
            return false;
        }
    }
    ///读“资产”表
    if (isFetchAsset || isFetchAccount) {
        if (fetchAsset()) {
            doSortPosition(false, true);
        } else {
            return false;
        }
    }
    return true;
}
bool myAccountAssetRootNode::callback(bool isRemoveAccount, bool isRemoveAsset) {
    qDebug() << "### myAccountAssetRootNode::callback ###";
    if (!isRemoveAccount && !isRemoveAsset) {
        return true;
    }
    int assetAccountCount = rootNode.children.count();
    for (int i = assetAccountCount-1; i >= 0; i--) {
        int assetHoldCount = rootNode.children.at(i)->children.count();
        for (int j = assetHoldCount-1; j >= 0; j--) {
            delete rootNode.children.at(i)->children.at(j);
        }
        rootNode.children.at(i)->children.clear();

        if (isRemoveAccount)
            delete rootNode.children.at(i);
    }
    if (isRemoveAccount)
        rootNode.children.clear();

    return true;
}
///读“资产帐户”表
bool myAccountAssetRootNode::fetchAccount() {
    qDebug() << "## myAccountAssetRootNode::fetchAccount ##";
    QSqlQuery query;
    if(query.exec(STR("select * from 资产帐户"))) {
        int i = 0;
        while(query.next()) { // 定位结果到下一条记录
            myAssetAccount tmpAccount;
            tmpAccount.accountData.code = query.value(0).toString();
            tmpAccount.accountData.name = query.value(1).toString();
            tmpAccount.accountData.type = query.value(2).toString();
            tmpAccount.accountData.note = query.value(3).toString();
            tmpAccount.pos  = query.value(4).toInt();
            if (tmpAccount.accountData.name.contains(STR("工商银行"))) {
                tmpAccount.logo = "gsyh.png";
            } else if (tmpAccount.accountData.name.contains(STR("招商银行"))) {
                tmpAccount.logo = "zsyh.png";
            } else if (tmpAccount.accountData.name == STR("中国银行")) {
                tmpAccount.logo = "zgyh.png";
            } else if (tmpAccount.accountData.name == STR("华泰证券")) {
                tmpAccount.logo = "htzq.png";
            } else if (tmpAccount.accountData.name == STR("国泰君安")) {
                tmpAccount.logo = "gtja.png";
            } else if (tmpAccount.accountData.name == STR("支付宝")) {
                tmpAccount.logo = "zfb.png";
            } else {
                tmpAccount.logo = "nologo";
            }

            QVariant data;
            data.setValue(tmpAccount);
            myAssetNode *account = new myAssetNode(myAssetNode::nodeAccount, data);
            account->parent = &rootNode;

            if (tmpAccount.accountData.code != OTHER_ACCOUNT)
                rootNode.addChild(account);

            i ++;
        }
    } else { // 如果查询失败，用下面的方法得到具体数据库返回的原因
        qDebug() << "Fetch Account Data to MySql error: " << query.lastError().text();
        return false;
    }
    query.clear();
    return true;
}
///读“资产”表
bool myAccountAssetRootNode::fetchAsset() {
    qDebug() << "## myAccountAssetRootNode::fetchAsset ##";
    QSqlQuery query;
    if(query.exec(STR("select * from 资产"))) {
        int i = 0;
        while(query.next()) { // 定位结果到下一条记录
            myAssetHold tmpHold;
            tmpHold.assetData.assetCode   = query.value(0).toString();
            tmpHold.assetData.assetName   = query.value(1).toString();
            tmpHold.assetData.accountCode = query.value(2).toString();
            tmpHold.assetData.amount      = query.value(3).toInt();
            tmpHold.assetData.price       = query.value(4).toFloat();
            tmpHold.assetData.type        = query.value(5).toString();

            tmpHold.pos                   = query.value(6).toInt();

            QVariant data;
            data.setValue(tmpHold);
            myAssetNode *hold = new myAssetNode(myAssetNode::nodeHolds, data);

            myAssetNode *account = getAccountNode(tmpHold.assetData.accountCode);
            if (nullptr == account) {
                continue;
            } else {
                hold->parent = account;
                account->addChild(hold);
                i++;
                //qDebug() << "account:" << tmpHold.assetData.accountCode << " \tasset:" << tmpHold.assetData.assetCode;
            }
        }
    } else { // 如果查询失败，用下面的方法得到具体数据库返回的原因
        qDebug() << "Fetch Asset Data to MySql error: " << query.lastError().text();
        return false;
    }
    query.clear();
    return true;
}

myAssetNode *myAccountAssetRootNode::getAccountNode(const QString &accountCode) const {
    for ( int i = 0; i != rootNode.children.size(); ++i ) {
        if ((rootNode.children.at(i)->nodeData).value<myAssetAccount>().accountData.code == accountCode ) {
            return rootNode.children.at(i);
        }
    }
    return nullptr;
}
myAssetNode *myAccountAssetRootNode::getAccountNode(int i) const {
    int count = rootNode.children.size();
    if (i < 0 || i >= count) {
        return nullptr;
    } else {
        return rootNode.children.at(i);
    }
}

QStringList myAccountAssetRootNode::getAllStockCodeList() {
    QStringList list;
    int numAccount = rootNode.children.size();
    for (int i = 0; i < numAccount; i++) {
        myAssetNode *account = rootNode.children.at(i);
        int numAsset = account->children.size();
        for (int j = 0; j < numAsset; j++) {
            myAssetNode *asset = account->children.at(j);
            list.append((asset->nodeData).value<myAssetHold>().assetData.assetCode);
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

bool myAccountAssetRootNode::doChangeAssetDirectly(const myAssetNode *node, changeType type, QVariant data) {
    QSqlQuery query;
    QString execWord, filter;

    if (myAssetNode::nodeAccount == node->type) {
        /// INSERT ASSET
        if (POP_INSERT == type) {
            myAssetData assetData = data.value<myAssetData>();

            filter   = STR("资产帐户代号='%1' AND 代号='%2'")
                            .arg(assetData.accountCode).arg(assetData.assetCode);
            execWord = STR("select count(*) from 资产 WHERE %1").arg(filter);
            if (0 == myFinanceDatabase::getQueryRows(execWord)) {
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
                    return false;
                }
            } else { return false;}
        /// MODIFY ACCOUNT
        } else if (POP_MODIFY == type) {
            QString accountCode = node->nodeData.value<myAssetAccount>().accountData.code;
            filter   = STR("代号='%1'").arg(accountCode);
            execWord = STR("select count(*) from 资产帐户 WHERE %1").arg(filter);
            if (1 == myFinanceDatabase::getQueryRows(execWord)) {
                myAccountData accountData = data.value<myAccountData>();
                execWord = STR("UPDATE 资产帐户 SET 代号='%1', 名称='%2', 类别='%3', 备注='%4' WHERE %5")
                        .arg(accountData.code).arg(accountData.name).arg(accountData.type).arg(accountData.note)
                        .arg(filter);
                qDebug() << execWord;
                if(!query.exec(execWord)) {
                    qDebug() << query.lastError().text();
                    return false;
                } else { return true;}
            } else { return false;}
        /// DELETE ACCOUNT
        } else if (POP_DELETE == type) {
            QString accountCode = node->nodeData.value<myAssetAccount>().accountData.code;
            // delete holds
            int count = node->children.count();
            for (int i = count-1; i >= 0; i--) {
                QString assetCode = node->children.at(i)->nodeData.value<myAssetHold>().assetData.assetCode;
                qDebug() << "delete " << i << "@total:" << node->children.count();
                if (!deleteOneAsset(accountCode, assetCode)) {
                    return false;
                }
            }
            // delete account
            filter   = STR("代号='%1'").arg(accountCode);
            execWord = STR("select count(*) from 资产帐户 WHERE %1").arg(filter);
            qDebug() << execWord;
            if(!query.exec(execWord)) {
                return false;
            }
            query.next();
            if (1 == query.value(0).toInt()) {
                execWord = STR("select * from 资产帐户 WHERE %1").arg(filter);
                qDebug() << execWord;
                if(query.exec(execWord)) {
                    query.next();
                    int pos  = query.value(4).toInt();

                    execWord = STR("delete from 资产帐户 WHERE %1").arg(filter);
                    qDebug() << execWord;
                    if(query.exec(execWord)) {
                        int toRemove = -1;
                        for (int i = 0; i < rootNode.children.count(); i++) {
                            const myAssetAccount &tmpAccount = rootNode.children.at(i)->nodeData.value<myAssetAccount>();
                            if (tmpAccount.accountData.code == accountCode) {
                                toRemove = i;
                            } else if (tmpAccount.pos > pos) {
                                if (!setAccountPosition(tmpAccount.accountData.code, tmpAccount.pos-1)) {
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
        QString originalAccountCode = node->nodeData.value<myAssetHold>().assetData.accountCode;
        QString originalAssetCode = node->nodeData.value<myAssetHold>().assetData.assetCode;
        /// MODIFY ASSET
        if (POP_MODIFY == type) {
            myAssetData assetData = data.value<myAssetData>();

            filter   = STR("资产帐户代号='%1' AND 代号='%2'")
                            .arg(originalAccountCode).arg(originalAssetCode);
            execWord = STR("select count(*) from 资产 WHERE %1").arg(filter);
            qDebug() << execWord;
            if (1 == myFinanceDatabase::getQueryRows(execWord)) {
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
        /// DELETE ASSET
        } else if (POP_DELETE == type) {
            // delete holds
            return deleteOneAsset(originalAccountCode, originalAssetCode);
        }
    } else { return false;}
    return false;
}
bool myAccountAssetRootNode::deleteOneAsset(const QString &accountCode, const QString &assetCode) {
    QSqlQuery query;
    QString filter   = STR("资产帐户代号='%1' AND 代号='%2'").arg(accountCode).arg(assetCode);
    QString execWord = STR("select * from 资产 WHERE %1").arg(filter);
    qDebug() << execWord;
    if(query.exec(execWord)) {
        execWord = STR("select count(*) from 资产 WHERE %1").arg(filter);
        if (1 == myFinanceDatabase::getQueryRows(execWord)) {
            query.next();
            int pos = query.value(6).toInt();

            execWord = STR("delete from 资产 WHERE %1").arg(filter);
            qDebug() << execWord;
            if(query.exec(execWord)) {
                myAssetNode *tmpAccount = getAccountNode(accountCode);
                int toRemove = -1;
                for (int i = 0; i < tmpAccount->children.count(); i++) {
                    const myAssetHold &tmpHold = tmpAccount->children.at(i)->nodeData.value<myAssetHold>();
                    if (tmpHold.assetData.assetCode == assetCode) {
                        toRemove = i;
                    } else if (tmpHold.pos > pos) {
                        if (!setAssetPosition(accountCode, tmpHold.assetData.assetCode, tmpHold.pos-1)) {
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
bool myAccountAssetRootNode::doInsertAccount(myAccountData data) {
    QSqlQuery query;
    QString execWord, filter;
    filter   = STR("代号='%1'").arg(data.code);
    execWord = STR("select count(*) from 资产帐户 WHERE %1").arg(filter);
    if (0 == myFinanceDatabase::getQueryRows(execWord)) {
        execWord = STR("INSERT INTO 资产帐户 VALUES ('%1', '%2', '%3', '%4', %5)")
                .arg(data.code).arg(data.name).arg(data.type).arg(data.note).arg(rootNode.children.count());
        qDebug() << execWord;
        if(query.exec(execWord)) {
            myAssetAccount tmpAccount(data);
            QVariant tmpData;
            tmpData.setValue(tmpAccount);
            myAssetNode *account = new myAssetNode(myAssetNode::nodeAccount, tmpData);
            account->parent = &rootNode;

            rootNode.addChild(account);
            return true;
        } else {
            qDebug() << query.lastError().text();
            return false;
        }
    } else { return false;}
    return false;
}

/// POSITION
bool myAccountAssetRootNode::setAccountPosition(const QString &accountCode, int pos) {
    QSqlQuery query;
    QString filter   = STR("代号='%1'").arg(accountCode);
    QString execWord = STR("select count(*) from 资产帐户 WHERE %1").arg(filter);
    if(1 == myFinanceDatabase::getQueryRows(execWord)) {
        execWord = STR("UPDATE 资产帐户 SET pos='%1' WHERE %2")
                .arg(pos).arg(filter);
        qDebug() << execWord;
        if(query.exec(execWord)) {
            for (int i = 0; i < rootNode.children.count(); i++) {
                myAssetAccount tmpAccount = rootNode.children.at(i)->nodeData.value<myAssetAccount>();
                if (tmpAccount.accountData.name == accountCode) {
                    tmpAccount.pos = pos;
                    break;
                }
            }
            return true;
        } else {
            qDebug() << query.lastError().text();
            return false;}
    } else { return false;}
}
bool myAccountAssetRootNode::setAssetPosition(const QString &accountCode, const QString &assetCode, int pos) {
    QSqlQuery query;
    QString filter   = STR("资产帐户代号='%1' AND 代号='%2'").arg(accountCode).arg(assetCode);
    QString execWord = STR("select count(*) from 资产 WHERE %1").arg(filter);
        if (1 == myFinanceDatabase::getQueryRows(execWord)) {
        execWord = STR("UPDATE 资产 SET pos='%1' WHERE %2").arg(pos).arg(filter);
        qDebug() << execWord;
        if(query.exec(execWord)) {
            myAssetNode *tmpAccount = getAccountNode(accountCode);
            for (int i = 0; i < tmpAccount->children.count(); i++) {
                myAssetHold tmpHold = tmpAccount->children.at(i)->nodeData.value<myAssetHold>();
                if (tmpHold.assetData.accountCode == assetCode) {
                    tmpHold.pos = pos;
                    tmpAccount->children.at(i)->nodeData.setValue(tmpHold);
                    break;
                }
            }
            return true;
        } else {
            qDebug() << query.lastError().text();
            return false;
        }
    } else { return false;}
}

void myAccountAssetRootNode::doSortPosition(bool isSortAccount, bool isSortAsset) {
    /// SORT ACCOUNT
    if (isSortAccount) {
        sortPositionAccount();
    }
    /// SORT ASSET
    if (isSortAsset) {
        int numOfAccount = rootNode.children.count();
        for (int i = 0; i < numOfAccount; i++) {
            sortPositionAsset(rootNode.children.at(i));
        }
    }
}
void myAccountAssetRootNode::sortPositionAccount() {
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
            if (setAccountPosition(tmpAccount.accountData.code, tmpAccount.pos)) {
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
void myAccountAssetRootNode::sortPositionAsset(myAssetNode *accountNode) {
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
            if (setAssetPosition(tmpHold.assetData.accountCode, tmpHold.assetData.assetCode, tmpHold.pos)) {
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
