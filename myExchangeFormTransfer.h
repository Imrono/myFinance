﻿#ifndef MYEXCHANGEFORMTRANSFER_H
#define MYEXCHANGEFORMTRANSFER_H
#include "myExchangeFormTabBase.h"

namespace Ui {
class myExchangeFormTransfer;
}

class myExchangeFormTransfer : public myExchangeFormTabBase
{
    Q_OBJECT

public:
    explicit myExchangeFormTransfer(const myAccountAssetRootNode *rootNode, QString tabName, QWidget *parent = 0, bool isModifyExchange = false);
    ~myExchangeFormTransfer();

    void exchangeWindowFeeChanged(double fee);
    void recordExchangeData(myExchangeData &tmpData);
    void setUI(const myExchangeData &exchangeData);
    void checkAndSetDisable(const myExchangeData &exchangeData);

private:
    Ui::myExchangeFormTransfer *ui;

    QMap<int, int> inIdx2AccountIdx;
    QMap<int, int> outIdx2AccountIdx;

    // data.money+fee 转出； data.price 转入。
    float remainMoneyOut;
    float totalMoneyOut;
    float remainMoneyIn;
    float totalMoneyIn;

    const myAccountNode *accountOut;
    const myAccountNode *accountIn;

private slots:
    void on_moneyAccountOut_currentIndexChanged(int index);
    void on_moneyAccountIn_currentIndexChanged(int index);

    void on_moneyTransferSpinBox_valueChanged(double value);
};

#endif // MYEXCHANGEFORMTRANSFER_H
