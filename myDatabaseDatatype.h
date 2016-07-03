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

enum SHOW_CATEGORY {
    STOCK_CATEGORY        = 0,
    MONEY_MANAGE_CATEGORY = 1,
    FUND_CATEGORY         = 2
};

struct myAssetData;
int getShowCategory(const myAssetData &tmpAssetHold);

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

struct myAccountNodeData;
struct myAssetNodeData;
struct myAccountData {
    myAccountData();
    myAccountData(myAccountNodeData data);
    void DEBUG_TRACE();
    QString code;
    QString name;
    QString type;
    QString note;
    static bool isSameAccountData(const myAccountData &data1, const myAccountData &data2);
};
struct myAssetData {
    myAssetData();
    myAssetData(myAssetNodeData data);
    myAssetData &operator =(const myAssetData &data);
    bool operator ==(const myAssetData &data);
    myAssetData operator +(const myAssetData &data);
    void reset();
    void initMoneyAsset(const QString &accountCode, const float money);
    void DEBUG_TRACE();

    QString accountCode;
    QString assetCode;
    QString assetName;
    double  amount; //double keeps the precise of int
    float   price;
    QString type;
    static bool isSameAssetData(const myAssetData &data1, const myAssetData &data2);
};

//Q_DECLARE_METATYPE(myAccountData)
//Q_DECLARE_METATYPE(myAssetData)

struct myExchangeData {
    myExchangeData();
    myExchangeData operator -();
    myExchangeData &operator =(const myExchangeData &data);
    bool operator ==(const myExchangeData &data);
    void DEBUG_TRACE();

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
        TAB_MONUP = 4,
        TAB_FUNDS = 5,
        MAX_TAB_COUNT = 6
    };

    myExchangeUI() {
        numOfTabs = MAX_TAB_COUNT;
        tabType = TAB_NONE;
        isShowRollback = false;
    }
    myExchangeUI(const myExchangeData &data, bool isShowRollback)
        : isShowRollback(isShowRollback), exchangeData(data) {
        setTypeOfExchange(data);
    }

    int getNumOfTabs() const { return numOfTabs;}
    int getTabType() const { return tabType;}
    bool getIsShowRollback() const { return isShowRollback;}
    const myExchangeData &getExchangeData() const { return exchangeData;}

private:
    int numOfTabs;
    int tabType;
    bool isShowRollback;
    myExchangeData exchangeData;

    void setTypeOfExchange (const myExchangeData &data) {
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
            tabType = myExchangeUI::TAB_MONUP;
        } else if (data.exchangeType == STR("基金")) {
            tabType = myExchangeUI::TAB_FUNDS;
        } else {
            tabType = myExchangeUI::TAB_NONE;
        }
    }
};

struct myDividends {
    enum dividendType {
        UNSPECIFIED    = -1,
        STOCK_DIVIDEND = 0,
        INTRESTS       = 1,
        NO_RECORD      = 2
    };

    myDividends();
    QString toString() {
        return STR("时间：%1，转：%2，送：%3，派：%4，基数：%5，类型：%6").arg(time.toString())
                .arg(shareSplit).arg(shareBonus).arg(capitalBonus).arg(base).arg(type);
    }

    QDate time;
    float shareSplit;
    float shareBonus;
    float capitalBonus;
    int base;
    int type;
};

#endif // MYDATABASEDATATYPE

