#include "myMoneyIncomeExpenses.h"
#include "myFinanceDatabase.h"

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QDebug>
#include <QtAlgorithms>

myMoneyIncomeExpenses::myMoneyIncomeExpenses()
{

}


bool myMoneyIncomeExpenses::initIncomeExpensesType() {
    if (!myFinanceDatabase::isConnected) {
        if (!myFinanceDatabase::connectDB())
            return false;
    }

    QSqlQuery query;
    QString execWord = STR("select count(*) from 收支种类ui");
    qDebug() << execWord;
    int numOfTypes = myFinanceDatabase::getQueryRows(execWord);
    if (-1 == numOfTypes)   return false;
    int numOfIncome = 0, numOfExpenses = 0;

    execWord = STR("select * from 收支种类ui");
    qDebug() << execWord;
    if(query.exec(execWord)) {
        while(query.next()) {
            incomeExpensesUi tmp;
            tmp.name = query.value(0).toString();
            tmp.type = query.value(1).toString();
            tmp.note = query.value(2).toString();
            tmp.pos  = query.value(3).toInt();

            _totalType.append(tmp);
            if (tmp.type == STR("收入")) {
                numOfIncome++;
                _incomeType << tmp;
            } else if (tmp.type == STR("支出")) {
                numOfExpenses++;
                _expensesType << tmp;
            } else {}
        }
    } else {
        qDebug() << query.lastError().text();
        return false;
    }

    qSort(_incomeType.begin()  , _incomeType.end()  , incomeExpensesUi::lessThan);
    qSort(_expensesType.begin(), _expensesType.end(), incomeExpensesUi::lessThan);
    return true;
}
