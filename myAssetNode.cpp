﻿#include "myAssetNode.h"

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlError>
#include <QtSql/QSqlRecord>
#include "myFinanceDatabase.h"
#include "myGlobal.h"

#include <QVariant>
#include <QDebug>

myAssetNode *myAccountAssetRootNode::getAssetNode(const myAccountNode * const account, const QString &assetCode) {
    if (myIndexShell::nodeAccount == account->type) {
        foreach (myIndexShell *assetNode, account->children) {
            if (myIndexShell::nodeHolds == assetNode->type) {
                if (static_cast<myAssetNode *>(assetNode)->dbAssetData.assetData.assetCode == assetCode)
                    return static_cast<myAssetNode *>(assetNode);
            }
        }
    }
    return nullptr;
}

bool myAccountAssetRootNode::doExchange(const myAssetData &assetData) {
    myAccountNode *account = getAccountNode(assetData.accountCode);
    if (!account)
        return false;

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
        MY_DEBUG_SQL(execWord);
        if(query.exec(execWord)) {
            /// UPDATE MEMORY DATA
            myAssetNode *asset = myAccountAssetRootNode::getAssetNode(account, assetData.assetCode);
            if (asset) {    /// update MY_CASH
                myAssetNodeData &assetHold = asset->dbAssetData;
                assetHold.assetData.amount = assetData.amount;
                assetHold.assetData.price  = assetData.price;
                return true;
            }
        } else {
            MY_DEBUG_ERROR(query.lastError().text()+QString(" numRows=%1").arg(numRows));
            return false;
        }
    } else if (0 == numRows) {  ///INSERT
        execWord = STR("INSERT INTO 资产 VALUES ('%1', '%2', '%3', %4, %5, '%6', %7, %8)")
                .arg(assetData.assetCode).arg(assetData.assetName).arg(assetData.accountCode).arg(assetData.amount)
                .arg(assetData.price).arg(assetData.type).arg(account->children.count()).arg(-1);
        MY_DEBUG_SQL(execWord);
        if(query.exec(execWord)) {
            /// INSERT MEMORY DATA
            myAssetNodeData tmpAssetHold(assetData);
            tmpAssetHold.pos = getAccountNode(assetData.accountCode)->children.count();
            myAssetNode *asset = new myAssetNode(myIndexShell::nodeHolds, tmpAssetHold, account);
            account->addChild(asset);
            return true;
        } else {
            MY_DEBUG_ERROR(query.lastError().text()+QString(" numRows=%1").arg(numRows));
            return false;
        }
    } else {
        qDebug() << "大于一条记录 ERROR:" << execWord;
        return false;
    }
    return false;
}
bool myAccountAssetRootNode::checkExchange(const myExchangeData &data, QString &abnormalInfo) {
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
        MY_DEBUG_ERROR(query.lastError().text());
        abnormalCode = SQL_ERROR;
        return false;
    }
    query.clear();

    // 2 ASSET CHECK
    filter   = STR("资产帐户代号='%1' AND 代号='%2'").arg(data.assetData.accountCode).arg(data.assetData.assetCode);
    execWord = STR("select 数量, 单位成本 from 资产 WHERE %1").arg(filter);
    MY_DEBUG_SQL(execWord);
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
        MY_DEBUG_ERROR(query.lastError().text());
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

bool myAccountAssetRootNode::initial(bool isFetchAccount, bool isFetchAsset) {
    MY_A_TRACE("### myAccountAssetRootNode::initial  ###");
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
    MY_A_TRACE("### myAccountAssetRootNode::callback ###");
    if (!isRemoveAccount && !isRemoveAsset) {
        return true;
    }
    int numOfAccount = rootNode.children.count();
    for (int i = numOfAccount-1; i >= 0; i--) {
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
    QString execWord = STR("select * from 资产帐户");
    MY_DEBUG_SQL(execWord);
    if(query.exec(execWord)) {
        while(query.next()) { // 定位结果到下一条记录
            myAccountNodeData tmpAccountInfo;
            tmpAccountInfo.accountData.code = query.value(0).toString();
            tmpAccountInfo.accountData.name = query.value(1).toString();
            tmpAccountInfo.accountData.type = query.value(2).toString();
            tmpAccountInfo.accountData.note = query.value(3).toString();
            tmpAccountInfo.pos  = query.value(4).toInt();
            if (tmpAccountInfo.accountData.name.contains(STR("工商银行"))) {
                tmpAccountInfo.logo = "gsyh.png";
            } else if (tmpAccountInfo.accountData.name.contains(STR("招商银行"))) {
                tmpAccountInfo.logo = "zsyh.png";
            } else if (tmpAccountInfo.accountData.name == STR("中国银行")) {
                tmpAccountInfo.logo = "zgyh.png";
            } else if (tmpAccountInfo.accountData.name == STR("华泰证券")) {
                tmpAccountInfo.logo = "htzq.png";
            } else if (tmpAccountInfo.accountData.name == STR("国泰君安")) {
                tmpAccountInfo.logo = "gtja.png";
            } else if (tmpAccountInfo.accountData.name == STR("支付宝")) {
                tmpAccountInfo.logo = "zfb.png";
            } else {
                tmpAccountInfo.logo = "nologo";
            }

            myAccountNode *account = new myAccountNode(myIndexShell::nodeAccount, tmpAccountInfo, &rootNode);
            if (tmpAccountInfo.accountData.code != OTHER_ACCOUNT)
                rootNode.addChild(account);
        }
    } else { // 如果查询失败，用下面的方法得到具体数据库返回的原因
        qDebug() << "Fetch Account Data to MySql error: " << query.lastError().text();
        return false;
    }
    query.clear();
    qDebug() << STR("fetchAccount finished with %1 accounts").arg(rootNode.children.count());
    return true;
}
///读“资产”表
bool myAccountAssetRootNode::fetchAsset() {
    qDebug() << "## myAccountAssetRootNode::fetchAsset   ##";
    QSqlQuery query;
    QString execWord = STR("select * from 资产");
    MY_DEBUG_SQL(execWord);
    if(query.exec(execWord)) {
        int i = 0;
        while(query.next()) { // 定位结果到下一条记录
            myAssetNodeData tmpAssetHold;
            tmpAssetHold.assetData.assetCode   = query.value(0).toString();
            tmpAssetHold.assetData.assetName   = query.value(1).toString();
            tmpAssetHold.assetData.accountCode = query.value(2).toString();
            tmpAssetHold.assetData.amount      = query.value(3).toFloat();
            tmpAssetHold.assetData.price       = query.value(4).toFloat();
            tmpAssetHold.assetData.type        = query.value(5).toString();
            tmpAssetHold.pos                   = query.value(6).toInt();
            tmpAssetHold.category              = query.value(7).toInt();

            myAccountNode *account = getAccountNode(tmpAssetHold.assetData.accountCode);
            if (nullptr == account) {
                continue;
            } else {
                myAssetNode *asset = new myAssetNode(myIndexShell::nodeHolds, tmpAssetHold, account);
                account->addChild(asset);
                i++;
                //qDebug() << "account:" << tmpHold.assetData.accountCode << " \tasset:" << tmpHold.assetData.assetCode;
            }
        }
    } else { // 如果查询失败，用下面的方法得到具体数据库返回的原因
        MY_DEBUG_ERROR(STR("Fetch Asset Data to MySql error: %1").arg(query.lastError().text()));
        return false;
    }
    query.clear();
    return true;
}

myAccountNode *myAccountAssetRootNode::getAccountNode(const QString &accountCode) const {
    for ( int i = 0; i != rootNode.children.size(); ++i ) {
        myAccountNode *account = static_cast<myAccountNode *>(rootNode.children.at(i));
        if (account->dbAccountData.accountData.code == accountCode ) {
            return account;
        }
    }
    return nullptr;
}
myAccountNode *myAccountAssetRootNode::getAccountNode(int i) const {
    int count = rootNode.children.size();
    if (i < 0 || i >= count) {
        return nullptr;
    } else {
        return static_cast<myAccountNode *>(rootNode.children[i]);
    }
}

QStringList myAccountAssetRootNode::getAllStockCodeList() {
    QStringList list;
    int numAccount = rootNode.children.size();
    for (int i = 0; i < numAccount; i++) {
        const myAccountNode *account = static_cast<const myAccountNode *>(rootNode.children.at(i));
        int numAsset = account->children.size();
        for (int j = 0; j < numAsset; j++) {
            const myAssetNode *asset = static_cast<const myAssetNode *>(account->children.at(j));
            list.append(asset->dbAssetData.assetData.assetCode);
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

bool myAccountAssetRootNode::doChangeAssetDirectly(const myIndexShell *node, changeType type, void *data) {
    QSqlQuery query;
    QString execWord, filter;

    if (myIndexShell::nodeAccount == node->type) {
        const myAccountNode *account = static_cast<const myAccountNode *>(node);
        /// INSERT ASSET
        if (POP_INSERT == type) {
            myAssetData tmpAssetHold = *static_cast<myAssetData *>(data);

            filter   = STR("资产帐户代号='%1' AND 代号='%2'")
                            .arg(tmpAssetHold.accountCode).arg(tmpAssetHold.assetCode);
            execWord = STR("select count(*) from 资产 WHERE %1").arg(filter);
            if (0 == myFinanceDatabase::getQueryRows(execWord)) {
                myAccountNode *tmpAccount = getAccountNode(tmpAssetHold.accountCode);
                QString strPrice = QString::number(tmpAssetHold.price, 'f', 3);
                execWord = STR("INSERT INTO 资产 VALUES ('%1', '%2', '%3', %4, %5, '%6', %7, %8)")
                        .arg(tmpAssetHold.assetCode).arg(tmpAssetHold.assetName).arg(tmpAssetHold.accountCode)
                        .arg(tmpAssetHold.amount).arg(strPrice).arg(tmpAssetHold.type).arg(tmpAccount->children.count()).arg(-1);
                MY_DEBUG_SQL(execWord);
                if(query.exec(execWord)) {
                    myAssetNodeData tmpHold(tmpAssetHold);
                    myAssetNode *hold = new myAssetNode(myIndexShell::nodeHolds, tmpHold, tmpAccount);
                    tmpAccount->addChild(hold);
                    return true;
                } else {
                    MY_DEBUG_ERROR(query.lastError().text());
                    return false;
                }
            } else { return false;}
        /// MODIFY ACCOUNT
        } else if (POP_MODIFY == type) {
            QString accountCode = account->dbAccountData.accountData.code;
            filter   = STR("代号='%1'").arg(accountCode);
            execWord = STR("select count(*) from 资产帐户 WHERE %1").arg(filter);
            if (1 == myFinanceDatabase::getQueryRows(execWord)) {
                myAccountData tmpAccountInfo = *static_cast<myAccountData *>(data);
                execWord = STR("UPDATE 资产帐户 SET 代号='%1', 名称='%2', 类别='%3', 备注='%4' WHERE %5")
                        .arg(tmpAccountInfo.code).arg(tmpAccountInfo.name).arg(tmpAccountInfo.type).arg(tmpAccountInfo.note)
                        .arg(filter);
                MY_DEBUG_SQL(execWord);
                if(!query.exec(execWord)) {
                    MY_DEBUG_ERROR(query.lastError().text());
                    return false;
                } else { return true;}
            } else { return false;}
        /// DELETE ACCOUNT
        } else if (POP_DELETE == type) {
            QString accountCode = account->dbAccountData.accountData.code;
            // delete holds
            int count = account->children.count();
            for (int i = count-1; i >= 0; i--) {
                QString assetCode = static_cast<const myAssetNode *>(account->children.at(i))->dbAssetData.assetData.assetCode;
                qDebug() << "delete " << i << "@total:" << node->children.count();
                if (!deleteOneAsset(accountCode, assetCode)) {
                    return false;
                }
            }
            // delete account
            filter   = STR("代号='%1'").arg(accountCode);
            execWord = STR("select count(*) from 资产帐户 WHERE %1").arg(filter);
            MY_DEBUG_SQL(execWord);
            if(!query.exec(execWord)) {
                return false;
            }
            query.next();
            if (1 == query.value(0).toInt()) {
                execWord = STR("select * from 资产帐户 WHERE %1").arg(filter);
                MY_DEBUG_SQL(execWord);
                if(query.exec(execWord)) {
                    query.next();
                    int pos  = query.value(4).toInt();

                    execWord = STR("delete from 资产帐户 WHERE %1").arg(filter);
                    MY_DEBUG_SQL(execWord);
                    if(query.exec(execWord)) {
                        int toRemove = -1;
                        for (int i = 0; i < rootNode.children.count(); i++) {
                            const myAccountNodeData &accountInfo = GET_CONST_ACCOUNT_NODE_DATA(rootNode.children.at(i));
                            if (accountInfo.accountData.code == accountCode) {
                                toRemove = i;
                            } else if (accountInfo.pos > pos) {
                                if (!setAccountPosition(accountInfo.accountData.code, accountInfo.pos-1)) {
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
    } else if (myIndexShell::nodeHolds == node->type) {
        const myAssetNode *asset = static_cast<const myAssetNode *>(node);
        QString originalAccountCode = asset->dbAssetData.assetData.accountCode;
        QString originalAssetCode   = asset->dbAssetData.assetData.assetCode;
        /// MODIFY ASSET
        if (POP_MODIFY == type) {
            myAssetData tmpAssetHold = *static_cast<myAssetData *>(data);

            filter   = STR("资产帐户代号='%1' AND 代号='%2'")
                            .arg(originalAccountCode).arg(originalAssetCode);
            execWord = STR("select count(*) from 资产 WHERE %1").arg(filter);
            if (1 == myFinanceDatabase::getQueryRows(execWord)) {
                QString strPrice  = QString::number(tmpAssetHold.price,  'f', 3);
                QString strAmount = QString::number(tmpAssetHold.amount, 'f', 3);
                execWord = STR("UPDATE 资产 SET 代号='%1', 名称='%2', 资产帐户代号='%3', 数量=%4, 单位成本=%5, 类别='%6' "
                                                  "WHERE %7")
                        .arg(tmpAssetHold.assetCode).arg(tmpAssetHold.assetName).arg(tmpAssetHold.accountCode)
                        .arg(strAmount).arg(strPrice).arg(tmpAssetHold.type)
                        .arg(filter);
                MY_DEBUG_SQL(execWord);
                if(!query.exec(execWord)) {
                    MY_DEBUG_ERROR(query.lastError().text());
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
    QString execWord = STR("select count(*) from 资产 WHERE %1").arg(filter);
    if (1 == myFinanceDatabase::getQueryRows(execWord)) {
        query.next();
        int pos = query.value(6).toInt();

        execWord = STR("delete from 资产 WHERE %1").arg(filter);
        MY_DEBUG_SQL(execWord);
        if(query.exec(execWord)) {
            myAccountNode *account = getAccountNode(accountCode);
            int toRemove = -1;
            for (int i = 0; i < account->children.count(); i++) {
                const myAssetNodeData &assetHold = GET_CONST_ASSET_NODE_DATA(account->children.at(i));
                if (assetHold.assetData.assetCode == assetCode) {
                    toRemove = i;
                } else if (assetHold.pos > pos) {
                    if (!setAssetPosition(accountCode, assetHold.assetData.assetCode, assetHold.pos-1)) {
                        return false;
                    }
                }
            }
            account->children.removeAt(toRemove);
            return true;
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
        MY_DEBUG_SQL(execWord);
        if(query.exec(execWord)) {
            myAccountNodeData tmpAccountInfo(data);
            myAccountNode *account = new myAccountNode(myIndexShell::nodeAccount, tmpAccountInfo, &rootNode);
            rootNode.addChild(account);
            return true;
        } else {
            MY_DEBUG_ERROR(query.lastError().text());
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
        MY_DEBUG_SQL(execWord);
        if(query.exec(execWord)) {
            for (int i = 0; i < rootNode.children.count(); i++) {
                myAccountNodeData &accountInfo = GET_ACCOUNT_NODE_DATA(rootNode.children.at(i));
                if (accountInfo.accountData.name == accountCode) {
                    accountInfo.pos = pos;
                    break;
                }
            }
            return true;
        } else {
            MY_DEBUG_ERROR(query.lastError().text());
            return false;}
    } else { return false;}
}
bool myAccountAssetRootNode::setAssetPosition(const QString &accountCode, const QString &assetCode, int pos) {
    QSqlQuery query;
    QString filter   = STR("资产帐户代号='%1' AND 代号='%2'").arg(accountCode).arg(assetCode);
    QString execWord = STR("select count(*) from 资产 WHERE %1").arg(filter);
        if (1 == myFinanceDatabase::getQueryRows(execWord)) {
        execWord = STR("UPDATE 资产 SET pos='%1' WHERE %2").arg(pos).arg(filter);
        MY_DEBUG_SQL(execWord);
        if(query.exec(execWord)) {
            const myAccountNode *account = getAccountNode(accountCode);
            for (int i = 0; i < account->children.count(); i++) {
                myAssetNodeData &assetHold = GET_ASSET_NODE_DATA(account->children.at(i));
                if (assetHold.assetData.accountCode == assetCode) {
                    assetHold.pos = pos;
                    break;
                }
            }
            return true;
        } else {
            MY_DEBUG_ERROR(query.lastError().text());
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
            sortPositionAsset(static_cast<myAccountNode *>(rootNode.children.at(i)));
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
        const myAccountNodeData &accountInfo = GET_CONST_ACCOUNT_NODE_DATA(rootNode.children.at(i));
        posList.append(accountInfo.pos);
        for (int j = 0; j < left.count(); j++) {
            if (accountInfo.pos == left.at(j)) {
                left.removeAt(j);
                break;
            }
        }
    }

    for (int i = 0; i < numOfAccount; i++) {
        myAccountNodeData &accountInfo = GET_ACCOUNT_NODE_DATA(rootNode.children.at(i));
        if (accountInfo.pos < 0 || accountInfo.pos > numOfAccount - 1 || posList.count(accountInfo.pos)>1 ) {
            int tmpPos = left.back();
            if (setAccountPosition(accountInfo.accountData.code, tmpPos)) {
                for (int j = 0; j < posList.count(); j++) {
                    if (posList.at(j) == tmpPos) {
                        posList.removeAt(j);
                        break;
                    }
                }
                posList.append(accountInfo.pos);
                left.pop_back();
                accountInfo.pos = tmpPos;
            } else {} //sql error
        } else {}
    }

    // 按排好的顺序更新rootNode的children
    QList<myIndexShell *> tmpChild;
    for (int i = 0; i < numOfAccount; i++) {
        for (int j = 0; j < numOfAccount; j++) {
            if (i == GET_CONST_ACCOUNT_NODE_DATA(rootNode.children.at(j)).pos) {
                tmpChild.append(rootNode.children.at(j));
                break;
            }
        }
    }
    rootNode.children = tmpChild;
}
void myAccountAssetRootNode::sortPositionAsset(myAccountNode *accountNode) {
    if (myIndexShell::nodeAccount != accountNode->type) {
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
        const myAssetNodeData &assetHold = GET_CONST_ASSET_NODE_DATA(accountNode->children.at(i));
        posList.append(assetHold.pos);
        for (int j = 0; j < left.count(); j++) {
            if (assetHold.pos == left.at(j)) {
                left.removeAt(j);
                break;
            }
        }
    }

    for (int i = 0; i < numOfAsset; i++) {
        myAssetNodeData &assetHold = GET_ASSET_NODE_DATA(accountNode->children[i]);
        if (assetHold.pos < 0 || assetHold.pos > numOfAsset - 1 || posList.count(assetHold.pos)>1 ) {
            int tmpPos = left.back();
            if (setAssetPosition(assetHold.assetData.accountCode, assetHold.assetData.assetCode, tmpPos)) {
                for (int j = 0; j < posList.count(); j++) {
                    if (posList.at(j) == assetHold.pos) {
                        posList.removeAt(j);
                        break;
                    }
                }
                posList.append(tmpPos);
                left.pop_back();
                assetHold.pos = tmpPos;
            } else {} //sql error
        } else {}
    }

    // 按排好的顺序更新accountNode的children
    numOfAsset = accountNode->children.count();
    QList<myIndexShell *> tmpChild;
    for (int i = 0; i < numOfAsset; i++) {
        for (int j = 0; j < numOfAsset; j++) {
            const myAssetNodeData &assetHold = GET_CONST_ASSET_NODE_DATA(accountNode->children.at(j));
            if (i == assetHold.pos) {
                tmpChild.append(accountNode->children.at(j));
                break;
            }
        }
    }
    accountNode->children = tmpChild;
}
