#ifndef MYASSETNODE_H
#define MYASSETNODE_H

#include <QList>
#include <QString>
#include <QVariant>
#include <QtSql/QSqlDatabase>

#include "myDatabaseDatatype.h"

struct myAssetAccount {
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

    float value;
};

struct myAssetHold {
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

    SQL_ERROR    =100
};

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
    myAssetNode *getAccountNode(QString accountCode);

    bool initial();
    bool callback();

    bool doExchange(const exchangeData data);
    bool checkExchange(const exchangeData &data, exchangeAbnomal &abnormalCode);

    nodeType type;
    QVariant nodeData;
    myAssetNode *parent;

    QList<myAssetNode *> children;
};

#endif // MYASSETNODE_H
