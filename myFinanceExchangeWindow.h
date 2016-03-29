#ifndef MYFINANCEEXCHANGEWINDOW_H
#define MYFINANCEEXCHANGEWINDOW_H

#include <QDialog>
#include <QWidget>
#include <QButtonGroup>

#include "myDatabaseDatatype.h"
#include "myAssetNode.h"
#include "myStockCodeName.h"

namespace Ui {
class myFinanceExchangeWindow;
}

class myFinanceExchangeWindow : public QDialog
{
    Q_OBJECT

public:
    explicit myFinanceExchangeWindow(QWidget *parent = 0);
    ~myFinanceExchangeWindow();

    exchangeData const &getExchangeData() { return data;}

private slots:
    void on_buttonBox_accepted();

    void on_amountSpinBox_valueChanged(int value);

    void on_priceSpinBox_valueChanged(double value);

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

    void on_feeRateSpinBox_valueChanged(double feeRate);

private:
    Ui::myFinanceExchangeWindow *ui;
    QButtonGroup *grpBuySell;
    QButtonGroup *grpMarket;
    exchangeData data;
    double buySellFlag;
    int dataSource;
	int lastRadioBuySell;
    double commisionRate;

    myStockCodeName *stockCode; //用于从code到name的推导

    void initial(const myRootAccountAsset &rootNode);
    void updateBuySell();
    void updateMarketInfo();
    void updataData();
    void updateExchangeFee();
    void updateExchangeType();
};

#endif // MYFINANCEEXCHANGEWINDOW_H
