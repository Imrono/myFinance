#ifndef MYASSETNODE_H
#define MYASSETNODE_H

#include <QList>
#include <QString>
#include <QVariant>
#include <QtSql/QSqlDatabase>

#include "myDatabaseDatatype.h"

struct myAssetAccount {
    myAssetAccount();
    myAssetAccount(myAccountData data);
    QString code;
    QString name;
//    enum accountType {
//        bankCard   = 0,
//        aliPay     = 1,
//        creditCard = 2,
//        security   = 3
//    };
    QString type;
    QString logo;
    QString note;

    float value;

    int pos;
};

struct myAssetHold {
    myAssetHold();
    myAssetHold(myAssetData data);
    QString assetCode;
    QString name;
    QString accountCode;
    int amount;
    float price;
//    enum assetType {
//        debt    = 0,
//        current = 1,
//        stock   = 2
//    };
    QString type;

    float currentPrice;
    float value;

    int pos;
};

Q_DECLARE_METATYPE(myAssetAccount)
Q_DECLARE_METATYPE(myAssetHold)

enum exchangeAbnomal {
    NORMAL = 0,
    LACK_MONEY_1 = 1,
    LACK_MONEY_2 = 2,
    LACK_STOCK   = 3,
    UN_UNIQUE_1  = 4,
    UN_UNIQUE_2  = 5,
    MONEY_ZERO   = 6,
    PRICE_ZERO   = 7,
    NO_MONEY_ATTRIBUTE = 8,

    SQL_ERROR    =100
};

enum changeType {
    POP_INSERT = 0,
    POP_MODIFY = 1,
    POP_DELETE = 2,
    BTN_INSERT = 3
};

class myRootAccountAsset;
class myAssetNode
{
public:
    enum nodeType {
        nodeAccount = 0,
        nodeHolds   = 1,
        nodeRoot    = 2
    };

    myAssetNode();
    myAssetNode(nodeType type, QVariant nodeData);
    ~myAssetNode();

    void addChild(myAssetNode *childNode);

    static bool doExchange(const myExchangeData data, const myRootAccountAsset &rootNode);
    static bool checkExchange(const myExchangeData &data, QString &abnormalInfo);

    nodeType type;
    QVariant nodeData;
    myAssetNode *parent;

    QList<myAssetNode *> children;

};

class myRootAccountAsset {
public:
    myRootAccountAsset();
    ~myRootAccountAsset();

    bool initial();
    bool callback();

    myAssetNode *getRootNode() const { return const_cast<myAssetNode *>(&rootNode);}
    myAssetNode *getAccountNode(QString accountCode) const;
    myAssetNode *getAccountNode(int i) const;
    int getAccountCount() const { return rootNode.children.count();}

    QStringList getAllStockCodeList();

    bool doChangeAssetDirectly(const myAssetNode *node, changeType type, QVariant data);
    bool doInsertAccount(myAccountData data);

    bool setAccountPosition(const QString &accountCode, int pos);
    bool setAssetPosition(const QString &accountCode, const QString &assetCode, int pos);

private:
    myAssetNode rootNode;

    bool deleteOneAsset(const QString &accountCode, const QString &assetCode);

    void doSortPosition();
    void sortPositionAccount();
    void sortPositionAsset(myAssetNode *accountNode);
};

#endif // MYASSETNODE_H
