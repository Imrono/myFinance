#ifndef MYDATABASEDATATYPE
#define MYDATABASEDATATYPE
#include "myGlobal.h"

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

struct myExchangeDetail {

};

struct myAssetAccount;
struct myAssetHold;
struct myAccountData {
    myAccountData();
    myAccountData(myAssetAccount data);
    QString code;
    QString name;
    QString type;
    QString note;
    static bool isSameAccountData(const myAccountData &data1, const myAccountData &data2);
};
struct myAssetData {
    myAssetData();
    myAssetData(myAssetHold data);
    myAssetData &operator =(const myAssetData &data);
    bool operator ==(const myAssetData &data);
    myAssetData operator +(const myAssetData &data);
    void reset();
    void initMoneyAsset(const QString &accountCode, const float money);

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

struct myExchangeData {
    myExchangeData();
    myExchangeData operator -();
    myExchangeData &operator =(const myExchangeData &data);
    bool operator ==(const myExchangeData &data);

    int       id;
    QDateTime time;
    float     fee;      //为正数，用减法

    QString   accountMoney;
    float     money;
    myAssetData assetData;

    QString   exchangeType;

    // additional data
    myExchangeDetail detail;
};
class myExchangeUI {
public:
    enum TAB_TYPES {
        TAB_NONE  = -1,
        TAB_STOCK = 0,
        TAB_TRANS = 1,
        TAB_INCOM = 2,
        TAB_EXPES = 3,
        TAB_FUNDS = 4,
        MAX_TAB_COUNT = 5
    };

    myExchangeUI() {
        numOfTabs = MAX_TAB_COUNT;
        tabType = TAB_NONE;
    }
    myExchangeUI(const myExchangeData &data) {
        setTypeOfExchange(data);
    }

    int getNumOfTabs() const { return numOfTabs;}
    int getTabType() const { return tabType;}
    const myExchangeData &getExchangeData() const { return exchangeData;}

private:
    int numOfTabs;
    int tabType;
    myExchangeData exchangeData;

    void setTypeOfExchange (const myExchangeData &data) {
        exchangeData = data;
        numOfTabs = 1;
        if (data.exchangeType.contains(STR("证券"))) {
            tabType = myExchangeUI::TAB_STOCK;
        } else if (data.exchangeType == STR("转帐")) {
            tabType = myExchangeUI::TAB_TRANS;
        } else if (data.exchangeType == STR("收入")) {
            tabType = myExchangeUI::TAB_INCOM;
        } else if (data.exchangeType == STR("支出")) {
            tabType = myExchangeUI::TAB_EXPES;
        } else if (data.exchangeType == STR("理财")) {
            tabType = myExchangeUI::TAB_FUNDS;
        } else {
            tabType = myExchangeUI::TAB_NONE;
        }
    }
};

struct myDividends {
    myDividends();
    float shareSplit;
    float shareBonus;
    float capitalBonus;
    int base;
};

#endif // MYDATABASEDATATYPE

