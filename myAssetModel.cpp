#include "myAssetModel.h"
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlError>
#include <QtDebug>
#include <QtGui/QIcon>

myAssetModel::myAssetModel(QObject *parent)
    : QAbstractItemModel(parent), rootNode(nullptr)
{
//    pWin = (myFinanceMainWindow *)parent;

    rootNode = new myAssetNode(myAssetNode::nodeRoot, "RootNode");
    if (!rootNode->initial())
        qDebug() << "ERROR @ initial rootNode";

    qDebugNodeData();
}

myAssetModel::~myAssetModel()
{
    bool ans = rootNode->callback();
    Q_UNUSED(ans);
    delete rootNode;
}


QModelIndex myAssetModel::index(int row, int column, const QModelIndex &parent) const {
    if (!rootNode || row < 0 || column < 0)
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
    return 2;
}

QVariant myAssetModel::data(const QModelIndex &index, int role) const {
    if (Qt::DisplayRole == role) {
        myAssetNode *node = nodeFromIndex(index);
        if (!node)
            return QVariant();

        if (index.column() == 0) {
            switch (node->type) {
            case myAssetNode::nodeRoot:
                return QVariant();
            case myAssetNode::nodeAccount: {
                myAssetAccount account = node->nodeData.value<myAssetAccount>();
                return QString("%1 %2").arg(account.code).arg(account.name);
            }
            case myAssetNode::nodeHolds: {
                myAssetHold holds = node->nodeData.value<myAssetHold>();
                return QString("%1 %2").arg(holds.assetCode).arg(holds.name);
            }
            default:
                return QString("Unknown");
            }
        } else if (index.column() == 1) {
            switch (node->type) {
            case myAssetNode::nodeRoot:
                return QVariant();
            case myAssetNode::nodeAccount:
                return QVariant();
            case myAssetNode::nodeHolds: {
                myAssetHold holds = node->nodeData.value<myAssetHold>();
                return QString("%1@%2").arg(holds.amount).arg(holds.price);
            }
            default:
                return QString("Unknown");
            }
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
    } else {
        return QVariant();
    }
    return QVariant();
}

QVariant myAssetModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0) {
            return tr("Node");
        } else if (section == 1) {
            return tr("Value");
        }
    }
    return QVariant();
}

myAssetNode *myAssetModel::nodeFromIndex(const QModelIndex &index) const
{
    if (index.isValid()) {
        return static_cast<myAssetNode *>(index.internalPointer());
    } else {
        return rootNode;
    }
}

void myAssetModel::doExchange(exchangeData data) {
    //刷新rootNode
    beginResetModel();
    rootNode->doExchange(data);
    bool ans = rootNode->callback();
    Q_UNUSED(ans);
    ans = rootNode->initial();
    Q_UNUSED(ans);
    endResetModel();

    qDebugNodeData();
}
void myAssetModel::doReflash() {
    beginResetModel();
    bool ans = rootNode->callback();
    Q_UNUSED(ans);
    ans = rootNode->initial();
    Q_UNUSED(ans);
    endResetModel();
}

void myAssetModel::qDebugNodeData()
{
    for (int i = 0; i < rootNode->children.count(); i++) {
        myAssetAccount pAccount = rootNode->children.at(i)->nodeData.value<myAssetAccount>();
        QString code = pAccount.code;
        QString name = pAccount.name;
        QString type = pAccount.type;

        int assetCount = rootNode->children.at(i)->children.count();
        qDebug() << code << name << type << "has hold count:" << assetCount;

        for (int j = 0; j < assetCount; j++) {
            myAssetHold pHold = rootNode->children.at(i)->children.at(j)->nodeData.value<myAssetHold>();
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
