#ifndef MYEXCHANGEFORMEXPENSES_H
#define MYEXCHANGEFORMEXPENSES_H

#include "myExchangeFormTabBase.h"

namespace Ui {
class myExchangeFormExpenses;
}

class myExchangeFormExpenses : public myExchangeFormTabBase
{
    Q_OBJECT

public:
    explicit myExchangeFormExpenses(const myAccountAssetRootNode *rootNode, QString tabName, QWidget *parent = 0);
    ~myExchangeFormExpenses();

    void recordExchangeData(myExchangeData &tmpData);
    void setUI(const myExchangeData &exchangeData);


private:
    Ui::myExchangeFormExpenses *ui;

    QMap<int, int> spendIdx2AccountIdx;

};

#endif // MYEXCHANGEFORMEXPENSES_H
