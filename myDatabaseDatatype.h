#ifndef MYDATABASEDATATYPE
#define MYDATABASEDATATYPE

#include <QDateTime>
#include <QString>
#include <QVariant>

#define CASH "cash"
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

struct exchangeData {
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

struct insertAccountData {
    QString Code;
    QString Name;
    QString Type;
    QString Note;
};
struct insertAssetData {
    QString accountCode;
    QString assetCode;
    QString assetName;
    int     amount;
    float   price;
    QString type;
};

Q_DECLARE_METATYPE(insertAssetData)

#endif // MYDATABASEDATATYPE

