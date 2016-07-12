#ifndef MYEXCHANGEFORMTABBASE_H
#define MYEXCHANGEFORMTABBASE_H

#include <QWidget>
#include <QString>
#include <QDebug>

#include "myAssetNode.h"
#include "myDatabaseDatatype.h"
class myFinanceExchangeWindow;

class myExchangeFormTabBase : public QWidget
{
public:
    explicit myExchangeFormTabBase(const myAccountAssetRootNode *rootNode, QString tabName, int tabType, QWidget *parent = 0, bool isModifyExchange = false);

    const QString &getTabText() { return tabName;}
    void setTabText(QString &text) { tabName = text;}
    void setDateTime(const QDateTime &dataTime) { data.time = dataTime;}
    void setModifyChange(bool isModifyChange) {this->isModifyExchange = isModifyChange;}
    void recoverTypeAndFee();
    virtual void recordExchangeData(myExchangeData &tmpData);
    virtual void setUI(const myExchangeData &exchangeData);
    virtual void checkAndSetDisable(const myExchangeData &exchangeData);
    virtual void disableAll();

    const myExchangeData &getExchangeData();
    const int getTabType() { return tabType;}
    virtual void exchangeWindowFeeChanged(double fee);

protected:
    ///METHOD
    void setExchangeWindowType(const QString &type);
    void setExchangeWindowFee(double fee);

    float getTotalMoney(int index);
    float getAssetValue() {
        return data.assetData.amount*data.assetData.price;
    }
    float getMoneyUsed() {
        /// data.assetData.amount 卖出为'-'，买入为'+'
        /// data.money            卖出为'+'，买入为'-'
        return -data.assetData.amount*data.assetData.price - data.fee;
    }

    /// DATA
    bool isModifyExchange;

    myFinanceExchangeWindow *parent;
    const myAccountAssetRootNode *rootNode;
    int tabType;
    QString tabName;

    myExchangeData data;
    float buySellFlag;  //buy 1.0, sell -1.0
    float totalMoney;
    float remainMoney;

    /// MODIFY DATA
    float usedMoneyBeforeModify;
    float usedFeeBeforeModify;
};

#endif // MYEXCHANGEFORMTABBASE_H
