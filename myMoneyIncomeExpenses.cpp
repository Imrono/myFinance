#include "myMoneyIncomeExpenses.h"
#include "myFinanceDatabase.h"

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QDebug>
#include <QtAlgorithms>

myMoneyIncomeExpenses::myMoneyIncomeExpenses(QWidget *parent) : parent(parent){
    initIncomeExpensesType();
    QFont boldFont;
    boldFont.setBold(true);
    /// INCOME
    icmGrp = new QButtonGroup(parent);
    for (int i = 0; i < _incomeType.count(); i++) {
        QRadioButton *btn = new QRadioButton(_incomeType[i].name, parent);
        icmGrp->addButton(incomeBtn[i]);
        icmGrp->setId(incomeBtn[i], i);
        incomeBtn << btn;
    }
    otherIncome = new QRadioButton(STR("其它收入"), parent);
    otherIncomeEdit = new QLineEdit(STR(""), parent);
    addIncomeBtn = new QPushButton(STR("+"), parent);
    icmGrp->addButton(otherIncome);
    icmGrp->setId(otherIncome, NUM_INCOME_TYPE_UI);
    icmGrp->setExclusive(true);           //设为互斥
    incomeLabel = new QLabel(STR("收入金额："), parent);
    incomeLabel->setFont(boldFont);
    incomeSpinBox = new QDoubleSpinBox(parent);

    /// EXPENSES
    xpsGrp = new QButtonGroup(parent);
    for (int i = 0; i < _expensesType.count(); i++) {
        QRadioButton *btn = new QRadioButton(_expensesType[i].name, parent);
        xpsGrp->addButton(expensesBtn[i]);
        xpsGrp->setId(expensesBtn[i], i);
        expensesBtn << btn;
    }
    otherExpenses = new QRadioButton(STR("其它支出"), parent);
    otherExpensesEdit = new QLineEdit(STR(""), parent);
    addExpensesBtn = new QPushButton(STR("+"), parent);
    xpsGrp->addButton(otherExpenses);
    xpsGrp->setId(otherExpenses, NUM_EXPENSES_TYPE_UI);
    xpsGrp->setExclusive(true);           //设为互斥
    expensesLabel = new QLabel(STR("支出金额："), parent);
    expensesLabel->setFont(boldFont);
    expensesSpinBox = new QDoubleSpinBox(parent);

    //signal and slot
    connect(addIncomeBtn  , SIGNAL(clicked()), this, SLOT(addIncomeType()));
    connect(addExpensesBtn, SIGNAL(clicked()), this, SLOT(addExpensesType()));
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

            QString IncomeOrExpenses = query.value(4).toString();
            if (IncomeOrExpenses == STR("收入")) {
                numOfIncome++;
                tmp.isIncome = true;
                _incomeType << tmp;
            } else if (IncomeOrExpenses == STR("支出")) {
                numOfExpenses++;
                tmp.isIncome = false;
                _expensesType << tmp;
            } else {}
            _totalType.append(tmp);
        }
    } else {
        qDebug() << query.lastError().text();
        return false;
    }

    qSort(_incomeType.begin()  , _incomeType.end()  , incomeExpensesUi::lessThan);
    qSort(_expensesType.begin(), _expensesType.end(), incomeExpensesUi::lessThan);
    return true;
}

void myMoneyIncomeExpenses::getIncomeExpensesLayouts(QGridLayout *incomeLayout, QGridLayout *expensesLayout) {
    /// INCOME
    incomeLayout->addWidget(incomeLabel, 0, 0);
    incomeLayout->addWidget(incomeSpinBox, 0, 1);
    int numOfIncomeType = _incomeType.count() < NUM_INCOME_TYPE_UI ? _incomeType.count() : NUM_INCOME_TYPE_UI;
    for (int i = 0; i < numOfIncomeType; i++) {
        incomeLayout->addWidget(incomeBtn[i], i+1, 0);
    }
    incomeLayout->addWidget(otherIncome, NUM_INCOME_TYPE_UI+1, 0);
    incomeLayout->addWidget(otherIncomeEdit, NUM_INCOME_TYPE_UI+1, 1);
    incomeLayout->addWidget(addIncomeBtn, NUM_INCOME_TYPE_UI+1, 2);

    /// EXPENSES
    expensesLayout->addWidget(expensesLabel, 0, 0);
    expensesLayout->addWidget(expensesSpinBox, 0, 1);
    int numOfExpensesType = _expensesType.count() < NUM_EXPENSES_TYPE_UI ? _expensesType.count() : NUM_EXPENSES_TYPE_UI;
    for (int i = 0; i < numOfExpensesType; i++) {
        expensesLayout->addWidget(expensesBtn[i], i/2+1, i%2);
    }
    expensesLayout->addWidget(otherExpenses, NUM_EXPENSES_TYPE_UI/2+1, 0);
    expensesLayout->addWidget(otherExpensesEdit, NUM_EXPENSES_TYPE_UI/2+1, 1);
    expensesLayout->addWidget(addExpensesBtn, NUM_EXPENSES_TYPE_UI/2+1, 2);
}

void myMoneyIncomeExpenses::addIncomeType() {
    qDebug() << "addIncomeType clicked";
}
void myMoneyIncomeExpenses::addExpensesType() {
    qDebug() << "addExpensesType clicked";
}
