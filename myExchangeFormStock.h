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
    explicit myExchangeFormStock(const myAccountAssetRootNode *rootNode, QString tabName, QWidget *parent = 0, bool isModifyExchange = false);
    ~myExchangeFormStock();

    void exchangeWindowFeeChanged(double fee);
    void recordExchangeData(myExchangeData &tmpData);
    void setUI(const myExchangeData &exchangeData);
    void checkAndSetDisable(const myExchangeData &exchangeData);

private:
    ///UI
    Ui::myExchangeFormStock *ui;

    QButtonGroup *grpBuySell;
    QButtonGroup *grpMarket;

    bool isShowRemainStock;

    ///DATA
    const myStockCodeName *stockCode;
    QMap<int, int> exchangeIdx2AccountIdx;
    double buySellFlag;
    double commisionRate;
    float remainMoney;
    float totalMoney;
    float bonusTax;
    const myAccountNode *accountNode;

    ///METHOD
    void updateBuySell();
    void updateMarketInfo();
    void updateExchangeFee();

    float calcMoney() {
        /// data.assetData.amount 卖出为'-'，买入为'+'
        /// data.money            卖出为'+'，买入为'-'
        return -static_cast<float>(data.assetData.amount) * data.assetData.price - data.fee - bonusTax;
    }

private slots:
    void on_codeLineEdit_textEdited(const QString &str);
    void on_nameLineEdit_textChanged(const QString &name);
    void on_nameLineEdit_editingFinished();

    void on_amountLineEdit_textChanged(const QString &str);
    void on_spinBoxPrice_valueChanged(double value);

    void on_radioBuy_clicked();
    void on_radioSell_clicked();

    void on_radioSH_clicked();
    void on_radioSZ_clicked();
    void on_radioOther_clicked();

    void on_moneyAccount_currentIndexChanged(int index);
    void on_moneySpinBox_valueChanged(double value);

    void on_bonusTaxSpinBox_valueChanged(double value);
    void on_moneySpinBoxTotal_valueChanged(double value);
};

#endif // MYEXCHANGEFORMSTOCK_H
