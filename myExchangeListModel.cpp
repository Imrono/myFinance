#include "myExchangeListModel.h"
#include "myFinanceDatabase.h"

#include <QDebug>

myExchangeListModel::myExchangeListModel(QObject *parent) : QStringListModel(parent) {
    doReflash();
}
myExchangeListModel::~myExchangeListModel() {
}

bool myExchangeListModel::doExchange(myExchangeData &exchangeData, bool isDelete, bool isSyncWithDb) {
    // 1 doExchange to DB
    beginResetModel();
    bool ans = exchangeNode.doExchange(exchangeData, isDelete);
    endResetModel();

    // 2 更新list，刷新
    if (isSyncWithDb)
        doReflash();

    return ans;
}
QString myExchangeListModel::updateStrFromExchangeData(const myExchangeData &exchangeData) const {
    QString exchangeStr;
    if (MY_CASH == exchangeData.assetData.assetCode && 1 == exchangeData.assetData.amount
    && STR("利息") != exchangeData.exchangeType && STR("分红") != exchangeData.exchangeType) {
        if (qAbs(exchangeData.assetData.price + exchangeData.money) > 0.0001f)
            qDebug() << STR("[转帐]") << exchangeData.accountMoney << " " << exchangeData.money
                     << "!="    << exchangeData.assetData.accountCode << " " << exchangeData.assetData.price;
        QString strMoney = QString::number(exchangeData.money, 'f', 2);
        exchangeStr = STR("[%1]%2->%3(￥%4)")
                .arg(exchangeData.exchangeType)
                .arg(exchangeData.accountMoney).arg(exchangeData.assetData.accountCode).arg(strMoney);
    } else if (STR("利息") == exchangeData.exchangeType || STR("分红") == exchangeData.exchangeType) {
        float money = 0.0f;
        if (STR("利息") == exchangeData.exchangeType) {
            money = exchangeData.assetData.price;
        } else if (STR("分红") == exchangeData.exchangeType) {
            money = exchangeData.money;
        } else { }
        exchangeStr = STR("[%1]%2.%3(￥%4)")
                .arg(exchangeData.exchangeType)
                .arg(exchangeData.accountMoney)
                .arg(exchangeData.assetData.assetName)
                .arg(money);
    } else {
        QString strMoney = QString::number(exchangeData.money, 'f', 2);
        QString strPrice = QString::number(exchangeData.assetData.price, 'f', 2);
        exchangeStr = STR("[%1]%2(￥%3) - %4@%5(%6*%7)")
                .arg(exchangeData.exchangeType)
                .arg(exchangeData.accountMoney).arg(strMoney)
                .arg(exchangeData.assetData.assetName).arg(exchangeData.assetData.accountCode)
                .arg(exchangeData.assetData.amount).arg(strPrice);
    }
    return exchangeStr;
}

void myExchangeListModel::doReflash() {
    beginResetModel();
    exchangeNode.callback();
    exchangeNode.initial();
    endResetModel();
}

void myExchangeListModel::coordinatorModifyExchange(const myExchangeData &originData, const myExchangeData &targetData, int &changeIdx) {
    changeIdx = NO_DO_EXCHANGE;

    if (originData.accountMoney != targetData.accountMoney) {
        changeIdx |= ROLLBACK_ACCOUNT_1;
        changeIdx |= TARG_ACCOUNT_1;
    }
    if (   originData.assetData.accountCode != targetData.assetData.accountCode
        || originData.assetData.assetCode != targetData.assetData.assetCode
        || (qAbs(originData.assetData.price - targetData.assetData.price) > MONEY_EPS)  //price会影响avgCost的计算
        || originData.assetData.amount != targetData.assetData.amount) {                //amount会影响avgCost的计算
        changeIdx |= ROLLBACK_ACCOUNT_2;
        changeIdx |= TARG_ACCOUNT_2;
    }
    if (qAbs(originData.money - targetData.money) > MONEY_EPS) {
        changeIdx |= TARG_ACCOUNT_1;
    }
    if (originData.assetData.assetName != targetData.assetData.assetName) {
        changeIdx |= TARG_ACCOUNT_2;
    }
    if (   originData.time != targetData.time
        || originData.exchangeType != targetData.exchangeType) {
        changeIdx |= OTHER_EXCHANGE;
    }
}

QModelIndex myExchangeListModel::index(int row, int column, const QModelIndex &parent) const {
    Q_UNUSED(parent);
    if (row < 0 || column < 0 || row >= exchangeNode.getRowCount())
        return QModelIndex();
    return createIndex(row, column, const_cast<myExchangeData *>(exchangeNode.getDataPtrFromRow(row)));
}
int myExchangeListModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return exchangeNode.getRowCount();
}
int myExchangeListModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return 1;
}
QVariant myExchangeListModel::data(const QModelIndex &index, int role) const {
    if (Qt::DisplayRole == role) {
        if (exchangeNode.getRowCount() > index.row() && index.row() >= 0) {
            return updateStrFromExchangeData(*static_cast<myExchangeData *>(index.internalPointer()));
        } else {
            return QVariant();
        }
    } else {
        return QVariant();
    }
    return QVariant();
}
