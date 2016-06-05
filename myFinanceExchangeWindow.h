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

#include "myExchangeFormStock.h"
#include "myExchangeFormTransfer.h"
#include "myExchangeFormIncome.h"
#include "myExchangeFormExpenses.h"

namespace Ui {
class myFinanceExchangeWindow;
}

class myFinanceExchangeWindow : public QDialog
{
    Q_OBJECT

public:
    enum EXCHANGE_WINDOW_TYPE {
        TYPE_NONE  = 0x00,
        TYPE_STOCK = 0x01,
        TYPE_TRANS = 0x02,
        TYPE_INCOM = 0x04,
        TYPE_EXPES = 0x08,
        TYPE_ALL   = 0xFF
    };

    explicit myFinanceExchangeWindow(QWidget *parent = 0, unsigned winType = TYPE_ALL);
    ~myFinanceExchangeWindow();

    myExchangeData const &getExchangeData() { return _currentTab->getExchangeData();}
    bool getIsRollback() { return isRollback;}
    void showRollback();
    void setUI(const myExchangeData &exchangeData, bool rollbackShow = false);

    void setExchangeWindowUiType(const QString type);
    void setExchangeWindowUiFee(double fee);
    void getCommonExchangeData(myExchangeData &tmpData);

private slots:
    void on_tabWidget_currentChanged(int index);

    void on_exchangeFeeSpinBox_valueChanged(double value);

    void on_checkBoxRollback_clicked();

    void on_timeDateTimeEdit_dateTimeChanged(const QDateTime &dateTime);

private:

    Ui::myFinanceExchangeWindow *ui;
    unsigned winType;
    QList<myExchangeFormTabBase *> _myTabs;
    myExchangeFormTabBase *_currentTab;
    QDateTime dateTime;

    int dataSource;

    bool isRollback;

    const myStockCodeName *stockCode; //用于从code到name的推导
    const myRootAccountAsset *rootNode;

    bool checkDataConsistence();
    ///tab2, tab3
    void initIncomeExpensesRadioButton();
    //myMoneyIncomeExpenses _myIncomeExpenses;
};

#endif // MYFINANCEEXCHANGEWINDOW_H
