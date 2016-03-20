#ifndef MYDATABASEDATATYPE
#define MYDATABASEDATATYPE

#include <QDateTime>
#include <QString>

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

#endif // MYDATABASEDATATYPE

