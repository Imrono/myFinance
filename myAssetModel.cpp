﻿#include "myAssetModel.h"
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlError>
#include <QtDebug>
#include <QtGui/QIcon>
#include <QFont>

myAssetModel::myAssetModel(QObject *parent)
    : QAbstractItemModel(parent), root()
{
    if (!root.initial())
        qDebug() << "ERROR @ initial rootNode";

    connect(&stockPrice, SIGNAL(updatePriceFinish()), this, SLOT(updatePriceFinish()));

    //qDebugNodeData();
}

myAssetModel::~myAssetModel()
{
    bool ans = root.callback();
    Q_UNUSED(ans);
}

QModelIndex myAssetModel::index(int row, int column, const QModelIndex &parent) const {
    if (row < 0 || column < 0)
        return QModelIndex();
    myIndexShell *parentNode = nodeFromIndex(parent);
    myIndexShell *childNode  = parentNode->children.value(row);
    if (!childNode)
        return QModelIndex();
    return createIndex(row, column, childNode);
}

QModelIndex myAssetModel::parent(const QModelIndex &child) const {
    myIndexShell *node = nodeFromIndex(child);
    if (!node)
        return QModelIndex();
    myIndexShell *parentNode = node->parent;
    if (!parentNode)
        return QModelIndex();
    myIndexShell *grandparentNode = parentNode->parent;
    if (!grandparentNode)
        return QModelIndex();

    int row = grandparentNode->children.indexOf(parentNode);
    return createIndex(row, 0, parentNode);
}

int myAssetModel::rowCount(const QModelIndex &parent) const {
    if (parent.column() > 0)
        return 0;
    myIndexShell *parentNode = nodeFromIndex(parent);
    if (!parentNode)
        return 0;
    return parentNode->children.count();
}

int myAssetModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    if (!stockPrice.isInit()) {
        return 2;
    }  else {
        return 4;
    }
}

QVariant myAssetModel::data(const QModelIndex &index, int role) const {
    if (Qt::DisplayRole == role) {
        myIndexShell *node = nodeFromIndex(index);
        if (!node)
            return QVariant();

        /// 第1列
        if (index.column() == 0) {
            switch (node->type) {
            case myIndexShell::nodeRoot:
                return QVariant();
            case myIndexShell::nodeAccount: {
                const myAccountNodeData &account = GET_ACCOUNT_NODE_DATA(node);
                QString code;
                if (account.accountData.name.contains(STR("银行"))) {
                    code = "**** **** " + account.accountData.code.right(4);
                } else {
                    code = account.accountData.code;
                }
                return QString("%1 %2").arg(code).arg(account.accountData.name);
            }
            case myIndexShell::nodeHolds: {
                const myAssetNodeData &holds = GET_ASSET_NODE_DATA(node);
                return QString("%1 %2").arg(holds.assetData.assetCode).arg(holds.assetData.assetName);
            }
            default:
                return QString("Unknown");
            }
        /// 第2列
        } else if (index.column() == 1) {
            switch (node->type) {
            case myIndexShell::nodeRoot:
                return QVariant();
            case myIndexShell::nodeAccount:
                return QVariant();
            case myIndexShell::nodeHolds: {
                const myAssetNodeData &holds = GET_ASSET_NODE_DATA(node);
                if (holds.assetData.assetCode == "cash" || holds.assetData.type == STR("货币基金")) {
                    QString strPrice = QString::number(holds.assetData.price, 'f', 2);
                    return QString("%1").arg(strPrice);
                } else {
                    QString strPrice = QString::number(holds.assetData.price, 'f', 3);
                    return QString("%1@%2").arg(holds.assetData.amount).arg(strPrice);
                }
            }
            default:
                return QString("Unknown");
            }
        /// 第3列
        } else if (index.column() == 2 && myIndexShell::nodeHolds == node->type && stockPrice.isInit()) {
            const myAssetNodeData &holds = GET_ASSET_NODE_DATA(node);
            if (holds.assetData.assetCode == "cash" || holds.assetData.type == STR("货币基金")) {
                return QVariant();
            } else {
                QString strPrice = QString::number(currentPrice(stockPrice.getStockPriceRt(), holds.assetData.assetCode), 'f', 2);
                return QString("%1").arg(strPrice);
            }
        /// 第4列
        } else if (index.column() == 3 && stockPrice.isInit()) {
            if (myIndexShell::nodeHolds == node->type) {
                const myAssetNodeData &holds = GET_ASSET_NODE_DATA(node);
                if (holds.assetData.assetCode == "cash" || holds.assetData.type == STR("货币基金")) {
                    QString strValue = QString::number(holds.assetData.price, 'f', 2);
                    return QString("%1").arg(strValue);
                } else {
                    float price = currentPrice(stockPrice.getStockPriceRt(), holds.assetData.assetCode);
                    float totalValue = static_cast<float>(holds.assetData.amount) * price;
                    return QString("%1").arg(totalValue);
                }
            } else if (myIndexShell::nodeAccount == node->type) {
                float totalValue = 0.0f;
                for ( int i = 0; i != node->children.size(); ++i ) {
                    const myAssetNodeData &holds = GET_ASSET_NODE_DATA(node->children.at(i));
                    if (holds.assetData.assetCode == "cash" || holds.assetData.type == STR("货币基金")) {
                        totalValue += holds.assetData.price;
                    } else {
                        float price = currentPrice(stockPrice.getStockPriceRt(), holds.assetData.assetCode);
                        totalValue += static_cast<float>(holds.assetData.amount) * price;
                    }
                }
                QString strValue = QString::number(totalValue, 'f', 2);
                return QString("%1").arg(strValue);
            } else {
                return QVariant();
            }

        } else {
            return QVariant();
        }
    } else if (Qt::DecorationRole == role) {
#ifdef QT_NO_DEBUG
        myAssetNode *node = nodeFromIndex(index);
        if (!node)
            return QVariant();

        if (index.column() == 0) {
            if (myIndexShell::nodeAccount == node->type) {
                return QIcon(QString(":/icon/finance/resource/icon/finance/%1").arg(node->nodeData.value<myAccountNodeData>().logo));
            }
        }
#endif
        return QVariant();
    } else if (Qt::FontRole == role) {
        if (index.column() == 2 || index.column() == 3) {
            return QFont(QString(), -1, QFont::Bold);
        }
    } else if (Qt::TextColorRole == role) {
        myIndexShell *node = nodeFromIndex(index);
        if (!node)
            return QVariant();

        if (index.column() == 3 && myIndexShell::nodeHolds == node->type && stockPrice.isInit()) {
            const myAssetNodeData &holds = GET_ASSET_NODE_DATA(node);
            if (holds.assetData.assetCode == "cash" || holds.assetData.type == STR("货币基金")) {
                return QColor(Qt::gray);
            } else {
                float price = currentPrice(stockPrice.getStockPriceRt(), holds.assetData.assetCode);
                if (price - holds.assetData.price > 0.0001f) {        //赚
                    return QColor(Qt::red);
                } else if (price - holds.assetData.price < 0.0001f) { //亏
                    return QColor(Qt::green);
                } else {
                    return QColor(Qt::gray);
                }
            }
        } else {
            return QVariant();
        }
    } else {
        return QVariant();
    }
    return QVariant();
}

QVariant myAssetModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0) {
            return STR("资产名称");
        } else if (section == 1) {
            return STR("持有数量");
        } else if (section == 2) {
            return STR("现价");
        } else if (section == 3) {
            return STR("总值");
        }
    }
    return QVariant();
}

myIndexShell *myAssetModel::nodeFromIndex(const QModelIndex &index) const
{
    if (index.isValid()) {
        return static_cast<myIndexShell *>(index.internalPointer());
    } else {
        return const_cast<myIndexShell *>(static_cast<myIndexShell *>(root.getRootNode()));
    }
}

float myAssetModel::currentPrice(const QMap<QString, sinaRealTimeData> *priceMap, const QString assetCode) const {
    QMap<QString, sinaRealTimeData>::const_iterator it = priceMap->find(assetCode);
    float currentPrice = 0.0f;
    while (it != priceMap->end() && it.key() == assetCode) {
        currentPrice = it.value().price;
        if (currentPrice < 0.0001f) {
            currentPrice = it.value().lastClose;
        }
        ++it;
    }
    return currentPrice;
}

/////////////////////////////////////////////////////////////////////
bool myAssetModel::doDividend(const myDividends &divident, const myAssetData &nodeAssetData, myExchangeData &exchangeData) {
    qDebug() << "### myAssetModel::doDividend ###";
    float divide = (divident.shareBonus + divident.shareSplit) / static_cast<float>(divident.base);
    float bonse  = divident.capitalBonus / static_cast<float>(divident.base);
    exchangeData.time.setDate(divident.time);
    exchangeData.accountMoney = nodeAssetData.accountCode;
    exchangeData.assetData    = nodeAssetData;

    if (myDividends::INTRESTS == divident.type) {
        exchangeData.money           = 0.0f;
        exchangeData.assetData.price = bonse;
    } else if (myDividends::STOCK_DIVIDEND == divident.type) {
        exchangeData.money            = bonse * nodeAssetData.amount;
        exchangeData.assetData.amount = divide * nodeAssetData.amount;
    } else { }
    return doExchange(exchangeData);
}

bool myAssetModel::doExchange(const myExchangeData &exchangeData, bool reflash) {
    qDebug() << "### myAssetModel::doExchange ###";
    bool ans = true;
    const myAccountNode *account = nullptr;
    const myAssetNode   *asset = nullptr;
    myAssetData originalAssetData;

    if (qAbs(exchangeData.money) > MONEY_EPS) {
        myAssetData moneyData;
        account = root.getAccountNode(exchangeData.accountMoney);
        if (account) {
            asset = myAccountAssetRootNode::getAssetNode(account, MY_CASH);
            if (asset) {    /// update MY_CASH
                originalAssetData = GET_CONST_ASSET_NODE_DATA(asset).assetData;
            } else { }      /// insert MY_CASH
            float money = originalAssetData.price + exchangeData.money;
            moneyData.initMoneyAsset(exchangeData.accountMoney, money);
            ans = root.doExchange(moneyData) && ans;
        } else { return false;}
    }

    account = nullptr;
    asset = nullptr;
    originalAssetData.reset();
    myAssetData assetData = exchangeData.assetData;
    account = root.getAccountNode(exchangeData.assetData.accountCode);
    if (account) {
        asset = myAccountAssetRootNode::getAssetNode(account, exchangeData.assetData.assetCode);
        if (asset) {    /// update MY_ASSET
            originalAssetData = GET_CONST_ASSET_NODE_DATA(asset).assetData;
        } else { }      /// insert MY_ASSET

        if (exchangeData.assetData.assetCode != MY_CASH) {
            assetData.amount = originalAssetData.amount + exchangeData.assetData.amount;
            if (0 != assetData.amount)
                assetData.price = (-exchangeData.money + originalAssetData.price*originalAssetData.amount)/static_cast<float>(assetData.amount);
            else {
                qDebug() << "ERROR: 0 == assetData.amount";
                return false;
            }
        } else {
            assetData.amount = 1;
            assetData.price = exchangeData.assetData.price + originalAssetData.price;
        }
        ans = root.doExchange(assetData) && ans;
    } else { return false;}

    if (reflash) {
        ans = doReflashData() && ans;
        //qDebugNodeData();
    }
    return ans;
}
bool myAssetModel::checkExchange(const myExchangeData &data, QString &abnormalInfo) {
    return myAccountAssetRootNode::checkExchange(data, abnormalInfo);
}

bool myAssetModel::doReflashData(bool isAccount, bool isAsset) {
    beginResetModel();
    bool ans = root.callback(isAccount, isAsset);
    ans = root.initial(isAccount, isAsset) && ans;
    endResetModel();
    return ans;
}
void myAssetModel::doReflash() {
    beginResetModel();
    endResetModel();
    emit priceDataReflashed();
}

void myAssetModel::doUpdatePrice() {
    beginResetModel();
    QStringList list = root.getAllStockCodeList();
    stockPrice.getStockPrice(list);
    endResetModel();
}
void myAssetModel::updatePriceFinish() {
    doReflash();
}
float myAssetModel::doGetTotalAsset() {
    float totalValue = 0.0f;
    if (stockPrice.isInit()) {
        for (int i = 0; i < root.getAccountCount(); i++) {
            const myAccountNode *tmpAccount = root.getAccountNode(i);
            for (int j = 0; j < tmpAccount->children.count(); j++) {
                const myAssetNodeData &holds = GET_CONST_ASSET_NODE_DATA(tmpAccount->children.at(j));
                if (holds.assetData.assetCode == "cash" ) {
                    totalValue += holds.assetData.price;
                } else if (holds.assetData.type == STR("货币基金")) {
                    totalValue += holds.assetData.price * holds.assetData.amount;
                }
                else {
                    float price = currentPrice(stockPrice.getStockPriceRt(), holds.assetData.assetCode);
                    totalValue += static_cast<float>(holds.assetData.amount) * price;
                }
            }
        }
    }

    return totalValue;
}

float myAssetModel::doGetSecurityAsset() {
    float securityAsset = 0.0f;
    if (stockPrice.isInit()) {
        for (int i = 0; i < root.getAccountCount(); i++) {
            const myAccountNode *tmpAccount = root.getAccountNode(i);
            for (int j = 0; j < tmpAccount->children.count(); j++) {
                if (GET_CONST_ACCOUNT_NODE_DATA(tmpAccount).accountData.type == STR("券商")) {
                    const myAssetNodeData &holds = GET_CONST_ASSET_NODE_DATA(tmpAccount->children.at(j));
                    if (holds.assetData.assetCode == "cash" ) {
                        securityAsset += holds.assetData.price;
                    } else {
                        float price = currentPrice(stockPrice.getStockPriceRt(), holds.assetData.assetCode);
                        securityAsset += static_cast<float>(holds.assetData.amount) * price;
                    }
                }
            }
        }
    }

    return securityAsset;
}

void myAssetModel::qDebugNodeData()
{
    for (int i = 0; i < root.getAccountCount(); i++) {
        const myAccountNodeData &account = GET_CONST_ACCOUNT_NODE_DATA(root.getAccountNode(i));
        QString code = account.accountData.code;
        QString name = account.accountData.name;
        QString type = account.accountData.type;

        int assetCount = root.getAccountNode(i)->children.count();
        qDebug() << code << name << type << "has hold count:" << assetCount;

        for (int j = 0; j < assetCount; j++) {
            const myAssetNodeData &holds = GET_CONST_ASSET_NODE_DATA(root.getAccountNode(i)->children.at(j));
            QString code        = holds.assetData.assetCode;
            QString name        = holds.assetData.assetName;
            QString accountCode = holds.assetData.accountCode;
            int amount          = holds.assetData.amount;
            float price         = holds.assetData.price;
            QString type        = holds.assetData.type;

            qDebug() << code << name << accountCode << amount << price << type;
        }
    }
}

bool myAssetModel::doChangeAssetDirectly(const myIndexShell *node, changeType type, void *data) {
    qDebug() << "myAssetModel";
    bool ans = root.doChangeAssetDirectly(node, type, data);
    doReflashData(myIndexShell::nodeAccount == type, myIndexShell::nodeHolds == type);
    return ans;
}
bool myAssetModel::doInsertAccount(myAccountData data) {
    beginResetModel();
    bool ans = root.doInsertAccount(data);
    ans = root.callback(true, false) && ans;
    ans = root.initial(true, false) && ans;
    endResetModel();
    return ans;
}

bool myAssetModel::doUpDown(bool isUp, const myIndexShell *node) {
    int pos = -1;
    int pos2 = -1;
    if (myIndexShell::nodeAccount == node->type) {
        pos = GET_CONST_ACCOUNT_NODE_DATA(node).pos;
        if (   (0 == pos && isUp == true)
            || (node->parent->children.count()-1 == pos && isUp == false)) {
            return false;
        }
    } else if (myIndexShell::nodeHolds == node->type) {
        pos = GET_CONST_ASSET_NODE_DATA(node).pos;
        if (   (0 == pos && isUp == true)
            || (node->parent->children.count()-1 == pos && isUp == false)) {
            return false;
        }
    } else { return false;}

    if (isUp) {
        node->parent->children.swap(pos, pos-1);
        pos2 = pos-1;
    } else {
        node->parent->children.swap(pos, pos+1);
        pos2 = pos+1;
    }

    bool ans = true;
    if (myIndexShell::nodeAccount == node->type) {
        root.setAccountPosition(GET_CONST_ACCOUNT_NODE_DATA(node).accountData.code, pos2);
        root.setAccountPosition(GET_CONST_ACCOUNT_NODE_DATA(node->parent->children.at(pos)).accountData.code, pos);
    } else if (myIndexShell::nodeHolds == node->type) {
        const myAssetNodeData &hold1 = GET_CONST_ASSET_NODE_DATA(node);
        const myAssetNodeData &hold2 = GET_CONST_ASSET_NODE_DATA(node->parent->children.at(pos));
        root.setAssetPosition(hold1.assetData.accountCode, hold1.assetData.assetCode, pos2);
        root.setAssetPosition(hold2.assetData.accountCode, hold2.assetData.assetCode, pos);
    } else { return false;}
    doReflashData(myIndexShell::nodeAccount == node->type, myIndexShell::nodeHolds == node->type);

    return ans;
}
