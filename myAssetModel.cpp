#include "myAssetModel.h"
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlError>
#include <QtDebug>
#include <QtGui/QIcon>
#include <QFont>

myAssetModel::myAssetModel(QObject *parent)
    : QAbstractItemModel(parent), rootNode(myAssetNode::nodeRoot, "RootNode")
{
    if (!rootNode.initial())
        qDebug() << "ERROR @ initial rootNode";

    connect(&stockPrice, SIGNAL(updatePriceFinish()), this, SLOT(updatePriceFinish()));

    qDebugNodeData();
}

myAssetModel::~myAssetModel()
{
    bool ans = rootNode.callback();
    Q_UNUSED(ans);
}

void myAssetModel::updatePriceFinish() {
    doReflash();
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
                    return QString("%1").arg(holds.price);
                } else {
                    return QString("%1@%2").arg(holds.amount).arg(holds.price);
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
                return QString("%1").arg(currentPrice(stockPrice.getStockPriceRt(), holds.assetCode));
            }
        /// 第4列
        } else if (index.column() == 3 && stockPrice.isInit()) {
            if (myAssetNode::nodeHolds == node->type) {
                myAssetHold holds = node->nodeData.value<myAssetHold>();
                if (holds.assetCode == "cash") {
                    return QString("%1").arg(holds.price);
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
                return QString("%1").arg(totalValue);
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
                return QIcon(QString(".//resource//icon//%1").arg(node->nodeData.value<myAssetAccount>().logo));
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
        return const_cast<myAssetNode *>(&rootNode);
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
bool myAssetModel::doExchange(const exchangeData data) {
    bool ans = true;
    //刷新rootNode
    beginResetModel();
    ans = ans && rootNode.doExchange(data);
    ans = ans && rootNode.callback();
    ans = ans && rootNode.initial();
    endResetModel();

    qDebugNodeData();
    return ans;
}
void myAssetModel::doReflashAssetData() {
    beginResetModel();
    bool ans = rootNode.callback();
    Q_UNUSED(ans);
    ans = rootNode.initial();
    Q_UNUSED(ans);
    endResetModel();
}
void myAssetModel::doUpdatePrice() {
    beginResetModel();
    QStringList list = stockPrice.getStockCodeList(&rootNode);
    Q_UNUSED(list);
    stockPrice.getStockPrice();
    endResetModel();
}
void myAssetModel::doReflash() {
    beginResetModel();
    endResetModel();
    emit priceDataReflashed();
}

void myAssetModel::qDebugNodeData()
{
    for (int i = 0; i < rootNode.children.count(); i++) {
        myAssetAccount pAccount = rootNode.children.at(i)->nodeData.value<myAssetAccount>();
        QString code = pAccount.code;
        QString name = pAccount.name;
        QString type = pAccount.type;

        int assetCount = rootNode.children.at(i)->children.count();
        qDebug() << code << name << type << "has hold count:" << assetCount;

        for (int j = 0; j < assetCount; j++) {
            myAssetHold pHold = rootNode.children.at(i)->children.at(j)->nodeData.value<myAssetHold>();
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
