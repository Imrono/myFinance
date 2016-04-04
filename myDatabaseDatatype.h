#ifndef MYDATABASEDATATYPE
#define MYDATABASEDATATYPE

#include <QDateTime>
#include <QString>
#include <QVariant>

#define MY_CASH            "cash"
#define OTHER_ACCOUNT   "myOtherAccount"
#define OTHER_ASSET     "myOtherAsset"

#define MONEY_EPS  0.0001f
enum BuyAndSell {
    BUY  = 0,
    SELL = 1
};
enum StockMarket {
    SH = 0,
    SZ = 1,
    OTHER = 2
};

enum databaseExchangeType {
    buyStock  = 0,
    sellStock = 1,
    salary    = 2,
    other     = 3,
    numOfType = 4
};

struct myExchangeData {
    myExchangeData();
    myExchangeData operator -();

    int       id;
    QDateTime time;
    QString   type;
    QString   account1;
    float     money;
    QString   account2;
    QString   code;
    QString   name;
    float     price;
    int       amount;
    bool      buySell;
    float     fee;
};

struct myAssetAccount;
struct myAssetHold;
struct myAccountData {
    myAccountData();
    myAccountData(myAssetAccount data);
    QString originCode;
    QString Code;
    QString Name;
    QString Type;
    QString Note;
    static bool isSameAccountData(const myAccountData &data1, const myAccountData &data2);
};
struct myAssetData {
    myAssetData();
    myAssetData(myAssetHold data);
    QString originAccountCode;
    QString originAssetCode;
    QString accountCode;
    QString assetCode;
    QString assetName;
    int     amount;
    float   price;
    QString type;
    static bool isSameAssetData(const myAssetData &data1, const myAssetData &data2);
};

Q_DECLARE_METATYPE(myAccountData)
Q_DECLARE_METATYPE(myAssetData)

#endif // MYDATABASEDATATYPE

