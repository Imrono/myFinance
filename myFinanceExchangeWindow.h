#ifndef MYFINANCEEXCHANGEWINDOW_H
#define MYFINANCEEXCHANGEWINDOW_H
#include "myGlobal.h"

#include <QDialog>
#include <QWidget>
#include <QButtonGroup>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>

#include <QMap>

#include "myDatabaseDatatype.h"
#include "myAssetNode.h"
#include "myStockCodeName.h"
#include "myMoneyIncomeExpenses.h"

namespace Ui {
class myFinanceExchangeWindow;
}

class myFinanceExchangeWindow : public QDialog
{
    Q_OBJECT

public:
    explicit myFinanceExchangeWindow(QWidget *parent = 0, bool isModifyExchange = false);
    ~myFinanceExchangeWindow();

    myExchangeData const &getExchangeData() { return data;}
    bool getIsRollback() { return isRollback;}
    void showRollback();
    void setUI(const myExchangeData &exchangeData, bool rollbackShow = false);

private slots:
    void on_buttonBox_accepted();

    void on_spinBoxAmount_valueChanged(int value);

    void on_spinBoxPrice_valueChanged(double value);

    void on_radioBuy_clicked();

    void on_radioSell_clicked();

    void on_tabWidget_currentChanged(int index);

    void on_moneyTransferSpinBox_valueChanged(double value);

    void on_exchangeFeeSpinBox_valueChanged(double value);

    void on_radioSH_clicked();

    void on_radioSZ_clicked();

    void on_radioOther_clicked();

    void on_codeLineEdit_textChanged(const QString &str);

    void on_codeLineEdit_editingFinished();

    void on_nameLineEdit_editingFinished();

    void on_radioSalary_clicked();

    void on_radioOtherIncome_clicked();

    void on_lineEditIncomeType_textChanged(const QString &str);

    void on_checkBoxRollback_clicked();

    void on_moneySpinBox_valueChanged(double value);

    void on_moneyAccount_currentIndexChanged(int index);

    void on_moneyAccountOut_currentIndexChanged(int index);

    void on_moneyAccountIn_currentIndexChanged(int index);

private:
    Ui::myFinanceExchangeWindow *ui;
    QButtonGroup *grpBuySell;
    QButtonGroup *grpMarket;
    QButtonGroup *grpIncomeType;
    myExchangeData data;
    double buySellFlag;
    int dataSource;
	int lastRadioBuySell;
    double commisionRate;
    float remainMoney;
    float totalMoney;
    float remainMoneyOut;
    float totalMoneyOut;
    float remainMoneyIn;
    float totalMoneyIn;

    bool isModifyExchange;
    bool isRollback;

    QMap<int, int> exchangeIdx2AccountIdx;
    QMap<int, int> inIdx2AccountIdx;
    QMap<int, int> outIdx2AccountIdx;
    QMap<int, int> incomeIdx2AccountIdx;
    QMap<int, int> spendIdx2AccountIdx;

    const myStockCodeName *stockCode; //用于从code到name的推导
    const myRootAccountAsset *rootNode;

    void initial(const myRootAccountAsset &rootNode);
    void updateBuySell();
    void updateMarketInfo();
    void updataData();
    void updateExchangeFee();
    void updateExchangeType();
    void updateIncomeType();

    float getTotalMoney(int index);

    QComboBox      *uiAccount1;
    QDoubleSpinBox *uiMoney;
    QComboBox      *uiAccount2;
    QLineEdit      *uiCode;
    QSpinBox       *uiAmount;
    QDoubleSpinBox *uiPrice;
    QLineEdit      *uiName;

    ///tab2, tab3
    void initIncomeExpensesRadioButton();
    myMoneyIncomeExpenses _myIncomeExpenses;
};

#endif // MYFINANCEEXCHANGEWINDOW_H
