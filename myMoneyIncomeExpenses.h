#ifndef MYMONEYINCOMEEXPENSES_H
#define MYMONEYINCOMEEXPENSES_H

#include <QList>
#include <QString>

#define NUM_INCOME_TYPE_UI      3
#define NUM_EXPENSES_TYPE_UI    6

struct incomeExpensesUi {
    QString name;
    QString type;
    QString note;
    int pos;
    static bool lessThan(const incomeExpensesUi &data1, const incomeExpensesUi &data2) {
        return data1.pos < data2.pos;
    }
};

class myMoneyIncomeExpenses
{
public:
    myMoneyIncomeExpenses();

    bool initIncomeExpensesType();
    const QList<incomeExpensesUi> &getIncomeType() {
        return _incomeType;
    }
    const QList<incomeExpensesUi> &getExpensesType() {
        return _expensesType;
    }

private:
    QList<incomeExpensesUi> _totalType;
    QList<incomeExpensesUi> _incomeType;
    QList<incomeExpensesUi> _expensesType;
};

#endif // MYMONEYINCOMEEXPENSES_H
