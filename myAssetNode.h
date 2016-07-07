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
    myAccountNodeData() : pos(-1) {}
    myAccountNodeData(const myAccountData &data)
        : accountData(data), pos(-1) {}
    QString toString() {
        return STR("*myAccountData* %1;logo:%2;pos:%3")
                .arg(accountData.toString()).arg(logo).arg(pos);
    }
    void getLogoFromName() {
        if (accountData.name.contains(STR("工商银行"))) {
            logo = "gsyh.png";
        } else if (accountData.name.contains(STR("招商银行"))) {
            logo = "zsyh.png";
        } else if (accountData.name == STR("中国银行")) {
            logo = "zgyh.png";
        } else if (accountData.name == STR("华泰证券")) {
            logo = "htzq.png";
        } else if (accountData.name == STR("国泰君安")) {
            logo = "gtja.png";
        } else if (accountData.name == STR("支付宝")) {
            logo = "zfb.png";
        } else {
            logo = "nologo";
        }
    }

    myAccountData accountData;
    QString logo;
    int pos;
};
struct myAssetNodeData {
    myAssetNodeData()
        : currentPrice(0.0f), pos(-1), category(0) {}
    myAssetNodeData(const myAssetData &data)
        : currentPrice(0.0f), pos(-1), category(0), assetData(data) {}
    QString toString() {
        return STR("*myAssetNodeData* %1;currentPrice:%2;pos:%3;category::%4")
                .arg(assetData.toString()).arg(currentPrice).arg(pos).arg(category);
    }

    myAssetData assetData;
    float currentPrice;
    int pos;
    int category;
};
struct myRootNodeData {
    myRootNodeData() : numOfAccount(0) {}
    QString toString() {
        return STR("*myRootNodeData* numOfAccount:%1").arg(numOfAccount);
    }

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
    } type;

    myIndexShell() : parent(nullptr) {}
    myIndexShell(nodeType type, myIndexShell *parent)
        : type(type), parent(parent) {}
    ~myIndexShell() {}

    myIndexShell *parent;
    QList<myIndexShell *> children;
    void addChild(myIndexShell *childNode) {
        children.append(childNode);
    }
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

    bool doChangeAsset(const myAssetData &assetData);
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
    bool insertOneAsset(const myAssetData &insertAssetHold);
    bool modifyOneAsset(const QString &originalAccountCode, const QString &originalAssetCode, const myAssetData &targetAssetHold);

    void doSortPosition(bool isSortAccount = true, bool isSortAsset = true);
    void sortPositionAccount();
    void sortPositionAsset(myAccountNode *accountNode);
};

#endif // MYASSETNODE_H
