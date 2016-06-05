#ifndef MYEXCHANGEFORMSTOCK_H
#define MYEXCHANGEFORMSTOCK_H

#include <QButtonGroup>
#include <QMap>

#include "myStockCodeName.h"
#include "myExchangeFormTabBase.h"

namespace Ui {
class myExchangeFormStock;
}

class myExchangeFormStock : public myExchangeFormTabBase
{
    Q_OBJECT

public:
    explicit myExchangeFormStock(const myRootAccountAsset *rootNode, QString tabName, QWidget *parent = 0);
    ~myExchangeFormStock();

    void exchangeWindowFeeChanged(double fee);
    void recordExchangeData(myExchangeData &tmpData);
    void setUI(const myExchangeData &exchangeData);

private:
    ///UI
    Ui::myExchangeFormStock *ui;

    QButtonGroup *grpBuySell;
    QButtonGroup *grpMarket;

    ///DATA
    const myStockCodeName *stockCode;
    QMap<int, int> exchangeIdx2AccountIdx;
    double buySellFlag;
    double commisionRate;
    float remainMoney;
    float totalMoney;

    ///METHOD
    void updateBuySell();
    void updateMarketInfo();
    void updateExchangeFee();

private slots:
    void on_codeLineEdit_textChanged(const QString &str);
    void on_codeLineEdit_editingFinished();
    void on_nameLineEdit_editingFinished();

    void on_spinBoxAmount_valueChanged(int value);
    void on_spinBoxPrice_valueChanged(double value);

    void on_radioBuy_clicked();
    void on_radioSell_clicked();

    void on_radioSH_clicked();
    void on_radioSZ_clicked();
    void on_radioOther_clicked();

    void on_moneyAccount_currentIndexChanged(int index);
    void on_moneySpinBox_valueChanged(double value);
};

#endif // MYEXCHANGEFORMSTOCK_H