#ifndef MYINCOMEFORM_H
#define MYINCOMEFORM_H
#include "myExchangeFormTabBase.h"

#include <QButtonGroup>

namespace Ui {
class myExchangeFormIncome;
}

class myExchangeFormIncome : public myExchangeFormTabBase
{
    Q_OBJECT

public:
    explicit myExchangeFormIncome(const myAccountAssetRootNode *rootNode, QString tabName, QWidget *parent = 0);
    ~myExchangeFormIncome();

    void recordExchangeData(myExchangeData &tmpData);
    void setUI(const myExchangeData &exchangeData);

private:
    Ui::myExchangeFormIncome *ui;

    QMap<int, int> incomeIdx2AccountIdx;
    QButtonGroup *grpIncomeType;

    /// METHOD
    void updateIncomeType();

private slots:
    void on_radioSalary_clicked();

    void on_radioOtherIncome_clicked();

    void on_lineEditIncomeType_textChanged(const QString &str);
};

#endif // MYINCOMEFORM_H
