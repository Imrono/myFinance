#ifndef MYMONEYINCOMEEXPENSES_H
#define MYMONEYINCOMEEXPENSES_H

#include <QList>
#include <QString>
#include <QWidget>
#include <QGridLayout>
#include <QButtonGroup>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QRadioButton>
#include <QLabel>
#include <QPushButton>

#define NUM_INCOME_TYPE_UI      3
#define NUM_EXPENSES_TYPE_UI    6

struct incomeExpensesUi {
    QString name;
    QString type;
    QString note;
    int pos;
    bool isIncome;
    static bool lessThan(const incomeExpensesUi &data1, const incomeExpensesUi &data2) {
        return data1.pos < data2.pos;
    }
};

class myMoneyIncomeExpenses : public QObject
{
    Q_OBJECT

public:
    myMoneyIncomeExpenses(QWidget *parent);

    const QList<incomeExpensesUi> &getIncomeType() {
        return _incomeType;
    }
    const QList<incomeExpensesUi> &getExpensesType() {
        return _expensesType;
    }
    void getIncomeExpensesLayouts(QGridLayout *incomeLayout, QGridLayout *expensesLayout);

private:
    QWidget *parent;
    QList<incomeExpensesUi> _totalType;
    QList<incomeExpensesUi> _incomeType;
    QList<incomeExpensesUi> _expensesType;

    ///INCOME
    QList<QRadioButton*> incomeBtn;
    QButtonGroup *icmGrp;
    QRadioButton *otherIncome;
    QLineEdit *otherIncomeEdit;
    QPushButton *addIncomeBtn;
    QLabel *incomeLabel;
    QDoubleSpinBox *incomeSpinBox;
    ///EXPENSE
    QList<QRadioButton*> expensesBtn;
    QButtonGroup *xpsGrp;
    QRadioButton *otherExpenses;
    QLineEdit *otherExpensesEdit;
    QPushButton *addExpensesBtn;
    QLabel *expensesLabel;
    QDoubleSpinBox *expensesSpinBox;

    bool initIncomeExpensesType();

private slots:
    void addIncomeType();
    void addExpensesType();
};

#endif // MYMONEYINCOMEEXPENSES_H
