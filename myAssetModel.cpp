#include "myAssetModel.h"
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

    qDebugNodeData();
}

myAssetModel::~myAssetModel()
{
    bool ans = root.callback();
    Q_UNUSED(ans);
}

QModelIndex myAssetModel::index(int row, int column, const QModelIndex &parent) const {
    if (row < 0 || column < 0)
        return QModelIndex();
    myAssetNode *parentNode = nodeFromIndex(parent);
    myAssetNode *childNode = parentNode->children.value(row);
    if (!childNode)
        return QModelIndex();
    return createIndex(row, column, childNode);
}

QModelIndex myAssetModel::parent(const QModelIndex &child) const {
    myAssetNode *node = nodeFromIndex(child);
    if (!node)
        return QModelIndex();
    myAssetNode *parentNode = node->parent;
    if (!parentNode)
        return QModelIndex();
    myAssetNode *grandparentNode = parentNode->parent;
    if (!grandparentNode)
        return QModelIndex();

    int row = grandparentNode->children.indexOf(parentNode);
    return createIndex(row, 0, parentNode);
}

int myAssetModel::rowCount(const QModelIndex &parent) const {
    if (parent.column() > 0)
        return 0;
    myAssetNode *parentNode = nodeFromIndex(parent);
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
        myAssetNode *node = nodeFromIndex(index);
        if (!node)
            return QVariant();

        /// 第1列
        if (index.column() == 0) {
            switch (node->type) {
            case myAssetNode::nodeRoot:
                return QVariant();
            case myAssetNode::nodeAccount: {
                myAssetAccount account = node->nodeData.value<myAssetAccount>();
                QString code;
                if (account.name.contains(QString::fromLocal8Bit("银行"))) {
                    code = "**** **** " + account.code.right(4);
                } else {
                    code = account.code;
                }
                return QString("%1 %2").arg(code).arg(account.name);
            }
            case myAssetNode::nodeHolds: {
                myAssetHold holds = node->nodeData.value<myAssetHold>();
                return QString("%1 %2").arg(holds.assetCode).arg(holds.name);
            }
            default:
                return QString("Unknown");
            }
        /// 第2列
        } else if (index.column() == 1) {
            switch (node->type) {
            case myAssetNode::nodeRoot:
                return QVariant();
            case myAssetNode::nodeAccount:
                return QVariant();
            case myAssetNode::nodeHolds: {
                myAssetHold holds = node->nodeData.value<myAssetHold>();
                if (holds.assetCode == "cash") {
                    QString strPrice = QString::number(holds.price, 'f', 2);
                    return QString("%1").arg(strPrice);
                } else {
                    QString strPrice = QString::number(holds.price, 'f', 3);
                    return QString("%1@%2").arg(holds.amount).arg(strPrice);
                }
            }
            default:
                return QString("Unknown");
            }
        /// 第3列
        } else if (index.column() == 2 && myAssetNode::nodeHolds == node->type && stockPrice.isInit()) {
            myAssetHold holds = node->nodeData.value<myAssetHold>();
            if (holds.assetCode == "cash") {
                return QVariant();
            } else {
                QString strPrice = QString::number(currentPrice(stockPrice.getStockPriceRt(), holds.assetCode), 'f', 2);
                return QString("%1").arg(strPrice);
            }
        /// 第4列
        } else if (index.column() == 3 && stockPrice.isInit()) {
            if (myAssetNode::nodeHolds == node->type) {
                myAssetHold holds = node->nodeData.value<myAssetHold>();
                if (holds.assetCode == "cash") {
                    QString strValue = QString::number(holds.price, 'f', 2);
                    return QString("%1").arg(strValue);
                } else {
                    float price = currentPrice(stockPrice.getStockPriceRt(), holds.assetCode);
                    float totalValue = static_cast<float>(holds.amount) * price;
                    return QString("%1").arg(totalValue);
                }
            } else if (myAssetNode::nodeAccount == node->type) {
                float totalValue = 0.0f;
                for ( int i = 0; i != node->children.size(); ++i ) {
                    myAssetHold holds = (node->children.at(i)->nodeData).value<myAssetHold>();
                    if (holds.assetCode == "cash" ) {
                        totalValue += holds.price;
                    } else {
                        float price = currentPrice(stockPrice.getStockPriceRt(), holds.assetCode);
                        totalValue += static_cast<float>(holds.amount) * price;
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
        myAssetNode *node = nodeFromIndex(index);
        if (!node)
            return QVariant();

        if (index.column() == 0) {
            if (myAssetNode::nodeAccount == node->type) {
                return QIcon(QString(":/icon/finance/resource/icon/finance/%1").arg(node->nodeData.value<myAssetAccount>().logo));
            }
        }
    } else if (Qt::FontRole == role) {
        if (index.column() == 2 || index.column() == 3) {
            return QFont(QString(), -1, QFont::Bold);
        }
    } else if (Qt::TextColorRole == role) {
        myAssetNode *node = nodeFromIndex(index);
        if (!node)
            return QVariant();

        if (index.column() == 3 && myAssetNode::nodeHolds == node->type && stockPrice.isInit()) {
            myAssetHold holds = node->nodeData.value<myAssetHold>();
            if (holds.assetCode == "cash") {
                return QColor(Qt::gray);
            } else {
                float price = currentPrice(stockPrice.getStockPriceRt(), holds.assetCode);
                if (price - holds.price > 0.0001f) {        //赚
                    return QColor(Qt::red);
                } else if (price - holds.price < 0.0001f) { //亏
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
            return QString::fromLocal8Bit("资产名称");
        } else if (section == 1) {
            return QString::fromLocal8Bit("持有数量");
        } else if (section == 2) {
            return QString::fromLocal8Bit("现价");
        } else if (section == 3) {
            return QString::fromLocal8Bit("总值");
        }
    }
    return QVariant();
}

myAssetNode *myAssetModel::nodeFromIndex(const QModelIndex &index) const
{
    if (index.isValid()) {
        return static_cast<myAssetNode *>(index.internalPointer());
    } else {
        return const_cast<myAssetNode *>(root.getRootNode());
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
bool myAssetModel::doExchange(const myExchangeData &data, bool reflash) {
    bool ans = myAssetNode::doExchange(data, root);
    if (reflash) {
        ans = doReflashAssetData() && ans;
        qDebugNodeData();
    }
    return ans;
}
bool myAssetModel::checkExchange(const myExchangeData &data, QString &abnormalInfo) {
    return myAssetNode::checkExchange(data, abnormalInfo);
}

bool myAssetModel::doReflashAssetData() {
    beginResetModel();
    bool ans = root.callback();
    ans = root.initial() && ans;
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
            myAssetNode *tmpAccount = root.getAccountNode(i);
            for (int j = 0; j < tmpAccount->children.count(); j++) {
                const myAssetHold &holds = tmpAccount->children.at(j)->nodeData.value<myAssetHold>();
                if (holds.assetCode == "cash" ) {
                    totalValue += holds.price;
                } else {
                    float price = currentPrice(stockPrice.getStockPriceRt(), holds.assetCode);
                    totalValue += static_cast<float>(holds.amount) * price;
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
            myAssetNode *tmpAccount = root.getAccountNode(i);
            for (int j = 0; j < tmpAccount->children.count(); j++) {
                if (tmpAccount->nodeData.value<myAssetAccount>().type == QString::fromLocal8Bit("券商")) {
                    const myAssetHold &holds = tmpAccount->children.at(j)->nodeData.value<myAssetHold>();
                    if (holds.assetCode == "cash" ) {
                        securityAsset += holds.price;
                    } else {
                        float price = currentPrice(stockPrice.getStockPriceRt(), holds.assetCode);
                        securityAsset += static_cast<float>(holds.amount) * price;
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
        myAssetAccount pAccount = root.getAccountNode(i)->nodeData.value<myAssetAccount>();
        QString code = pAccount.code;
        QString name = pAccount.name;
        QString type = pAccount.type;

        int assetCount = root.getAccountNode(i)->children.count();
        qDebug() << code << name << type << "has hold count:" << assetCount;

        for (int j = 0; j < assetCount; j++) {
            myAssetHold pHold   = root.getAccountNode(i)->children.at(j)->nodeData.value<myAssetHold>();
            QString code        = pHold.assetCode;
            QString name        = pHold.name;
            QString accountCode = pHold.accountCode;
            int amount          = pHold.amount;
            float price         = pHold.price;
            QString type        = pHold.type;

            qDebug() << code << name << accountCode << amount << price << type;
        }
    }
}

bool myAssetModel::doChangeAssetDirectly(const myAssetNode *node, changeType type, QVariant data) {
    qDebug() << "myAssetModel";
    bool ans = root.doChangeAssetDirectly(node, type, data);
    doReflashAssetData();
    return ans;
}
bool myAssetModel::doInsertAccount(myAccountData data) {
    bool ans = root.doInsertAccount(data);
    doReflashAssetData();
    return ans;
}

bool myAssetModel::doUpDown(bool isUp, myAssetNode *node) {
    bool ans = false;
    int pos = -1;
    int pos2 = -1;
    if (myAssetNode::nodeAccount == node->type) {
        pos = node->nodeData.value<myAssetAccount>().pos;
        if (   (0 == pos && isUp == true)
            || (node->parent->children.count()-1 == pos && isUp == false)) {
            return false;
        }
    } else if (myAssetNode::nodeHolds == node->type) {
        pos = node->nodeData.value<myAssetHold>().pos;
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

    if (myAssetNode::nodeAccount == node->type) {
        root.setAccountPosition(node->nodeData.value<myAssetAccount>().code, pos2);
        root.setAccountPosition(node->parent->children.at(pos)->nodeData.value<myAssetAccount>().code, pos);
        ans = true;
    } else if (myAssetNode::nodeHolds == node->type) {
        myAssetHold &hold1 = node->nodeData.value<myAssetHold>();
        myAssetHold &hold2 = node->parent->children.at(pos)->nodeData.value<myAssetHold>();
        root.setAssetPosition(hold1.accountCode, hold1.assetCode, pos2);
        root.setAssetPosition(hold2.accountCode, hold2.assetCode, pos);
        ans = true;
    } else { return false;}
    doReflashAssetData();
    return ans;
}
