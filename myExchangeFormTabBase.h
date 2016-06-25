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
    explicit myExchangeFormTabBase(const myRootAccountAsset *rootNode, QString tabName, int tabType, QWidget *parent = 0, bool isModifyExchange = false);

    const QString &getTabText() { return tabName;}
    void setTabText(QString &text) { tabName = text;}
    void setDateTime(const QDateTime &dataTime) { data.time = dataTime;}
    void setModifyChange(bool isModifyChange) {this->isModifyExchange = isModifyChange;}
    void recoverTypeAndFee();
    virtual void recordExchangeData(myExchangeData &tmpData);
    virtual void setUI(const myExchangeData &exchangeData);
    virtual void checkAndSetDisable(const myExchangeData &exchangeData);

    const myExchangeData &getExchangeData();
    const int getTabType() { return tabType;}
    virtual void exchangeWindowFeeChanged(double fee);

protected:
    ///METHOD
    void setExchangeWindowType(const QString &type);
    void setExchangeWindowFee(double fee);

    float getTotalMoney(int index);

    void traceExchangeData();

    /// DATA
    bool isModifyExchange;

    myFinanceExchangeWindow *parent;
    const myRootAccountAsset *rootNode;
    myExchangeData data;
    int tabType;

    QString tabName;

    /// MODIFY DATA
    float usedMoneyBeforeModify;
    float usedFeeBeforeModify;
};

#endif // MYEXCHANGEFORMTABBASE_H
