#ifndef MYASSETNODE_H
#define MYASSETNODE_H
#include "myGlobal.h"

#include <QList>
#include <QString>
#include <QVariant>
#include <QtSql/QSqlDatabase>

#include "myDatabaseDatatype.h"

// IN_PARA: myIndexShell *node
#define GET_CONST_ASSET_NODE_DATA(assetNode) \
    static_cast<const myAssetNode *>(assetNode)->dbAssetData
#define GET_ASSET_NODE_DATA(assetNode) \
    static_cast<myAssetNode *>(assetNode)->dbAssetData
#define GET_ACCOUNT_NODE_DATA(accountNode) \
    static_cast<myAccountNode *>(accountNode)->dbAccountData
#define GET_CONST_ACCOUNT_NODE_DATA(accountNode) \
    static_cast<const myAccountNode *>(accountNode)->dbAccountData
#define GET_ROOT_NODE_DATA(rootNode) \
    static_cast<myRootNode *>(rootNode)->rootNodeData


struct myAccountNodeData {
    myAccountNodeData() {}
    myAccountNodeData(const myAccountData &data)
        : accountData(data) {}

    myAccountData accountData;
    QString logo;
    int pos;
};
struct myAssetNodeData {
    myAssetNodeData() {}
    myAssetNodeData(const myAssetData &data)
        : assetData(data) {}

    myAssetData assetData;
    float currentPrice;
    int pos;
    int category;
};
struct myRootNodeData {
    myRootNodeData() : numOfAccount(0) {}
    int numOfAccount;
};

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

class myAccountAssetRootNode;
class myIndexShell
{
public:
    enum nodeType {
        nodeAccount = 0,
        nodeHolds   = 1,
        nodeRoot    = 2
    };
    myIndexShell() : parent(nullptr) {}
    myIndexShell(nodeType type, myIndexShell *parent)
        : type(type), parent(parent) {}
    ~myIndexShell() {}

    myIndexShell *parent;
    QList<myIndexShell *> children;
    void addChild(myIndexShell *childNode) {
        children.append(childNode);
    }

//private:
    nodeType type;
};
class myRootNode : public myIndexShell {
public:
    myRootNode(nodeType type, const myRootNodeData& data, myIndexShell *parent)
        : myIndexShell(type, parent), rootNodeData(data) {}
    myRootNodeData rootNodeData;
};
class myAccountNode : public myIndexShell {
public:
    myAccountNode(nodeType type, const myAccountNodeData& data, myIndexShell *parent)
        : myIndexShell(type, parent), dbAccountData(data) {}
    myAccountNodeData dbAccountData;
};
class myAssetNode : public myIndexShell {
public:
    myAssetNode(nodeType type, const myAssetNodeData& data, myIndexShell *parent)
        : myIndexShell(type, parent), dbAssetData(data) {}
    myAssetNodeData dbAssetData;
};

///
/// \brief The myAccountAssetRootNode class
/// 1. fetch/push data from/to the DataBase
/// 2. sort the data
///
class myAccountAssetRootNode {
    friend class myIndexShell;
    friend class myRootNode;
    friend class myAccountNode;
    friend class myAssetNode;
public:
    myAccountAssetRootNode()
        : rootNode(myIndexShell::nodeRoot, myRootNodeData(), nullptr) {}
    ~myAccountAssetRootNode() {
        callback();
    }

    bool initial(bool isFetchAccount = true, bool isFetchAsset = true);
    bool callback(bool isRemoveAccount = true, bool isRemoveAsset = true);

    myRootNode *getRootNode() const { return const_cast<myRootNode *>(&rootNode);}
    myAccountNode *getAccountNode(const QString &accountCode) const;
    myAccountNode *getAccountNode(int i) const;
    int getAccountCount() const { return rootNode.children.count();}
    static myAssetNode *getAssetNode(const myAccountNode * const account, const QString &assetCode);

    QStringList getAllStockCodeList();

    bool doExchange(const myAssetData &assetData);
    static bool checkExchange(const myExchangeData &data, QString &abnormalInfo);

    bool doChangeAssetDirectly(const myIndexShell *node, changeType type, void *data);
    bool doInsertAccount(myAccountData data);

    bool setAccountPosition(const QString &accountCode, int pos);
    bool setAssetPosition(const QString &accountCode, const QString &assetCode, int pos);

private:
    myRootNode rootNode;

    bool fetchAccount();
    bool fetchAsset();

    bool deleteOneAsset(const QString &accountCode, const QString &assetCode);

    void doSortPosition(bool isSortAccount = true, bool isSortAsset = true);
    void sortPositionAccount();
    void sortPositionAsset(myAccountNode *accountNode);
};

#endif // MYASSETNODE_H
