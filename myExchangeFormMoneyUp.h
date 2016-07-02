#ifndef MYEXCHANGEFORMMONEYUP_H
#define MYEXCHANGEFORMMONEYUP_H

#include <QButtonGroup>

#include "myExchangeFormTabBase.h"

class myFinanceMainWindow;
namespace Ui {
class myExchangeFormMoneyUp;
}

class myExchangeFormMoneyUp : public myExchangeFormTabBase
{
    Q_OBJECT

public:
    explicit myExchangeFormMoneyUp(const myAccountAssetRootNode *rootNode, QString tabName, QWidget *parent = 0);
    ~myExchangeFormMoneyUp();

    void exchangeWindowFeeChanged(double fee);
    void recordExchangeData(myExchangeData &tmpData);
    void setUI(const myExchangeData &exchangeData);
    void checkAndSetDisable(const myExchangeData &exchangeData);

private:
    float getAssetCodeValue(const QString &code);
    QString getCurrentAssetName() {
        QString name;
        if (currentAsset) {
            name = GET_CONST_ASSET_NODE_DATA(currentAsset).assetData.assetName;
        }
        return name;
    }
    void updateBuySell(int caller);

private slots:
    void on_moneyAccount_currentIndexChanged(int index);

    void on_radioSubscribing_clicked();

    void on_radioRedeming_clicked();

    void on_keepsSpinBox_valueChanged(double value);
    void on_usedSpinBox_valueChanged(double value);
    void on_remainSpinBox_valueChanged(double value);

    void on_moneySpinBoxTotal_valueChanged(double value);
    void on_moneySpinBox_valueChanged(double value);
    void on_moneySpinBoxRemain_valueChanged(double value);

    void on_codeLineEdit_textChanged(const QString &str);

    void on_typeBox_currentIndexChanged(int index);

    void on_nameLineEdit_textChanged(const QString &str);

    void on_doDividendButton_clicked();

    void on_checkBoxSoldAll_clicked();

private:
    enum OperationType {
        SUBSCRIBING = 0,
        //PURCHASING  = 1,
        REDEMING    = 1
    };

    enum UPDATE_MONEY {
        FROM_UNDEFINE          = -1,
        FROM_RADIO_SUBSCRIBING = 0,
        FROM_RADIO_PURCHASING  = 1,
        FROM_RADIO_REDEMING    = 2,
        FROM_KEEPS_ASSET       = 3,
        FROM_REAMIN_ASSET      = 4,
        FROM_EXCHANGE_ASSET    = 5,
        FROM_FEE_CHANGE        = 6
    };
    const QString updateMoney2String(unsigned source) {
        switch (source) {
        case FROM_RADIO_SUBSCRIBING:
            return "FROM_RADIO_SUBSCRIBING";
        case FROM_RADIO_PURCHASING:
            return "FROM_RADIO_PURCHASING ";
        case FROM_RADIO_REDEMING:
            return "FROM_RADIO_REDEMING   ";
        case FROM_KEEPS_ASSET:
            return "FROM_KEEPS_ASSET      ";
        case FROM_REAMIN_ASSET:
            return "FROM_REAMIN_ASSET     ";
        case FROM_EXCHANGE_ASSET:
            return "FROM_EXCHANGE_ASSET   ";
        case FROM_FEE_CHANGE:
            return "FROM_FEE_CHANGE       ";
        default:
            return "unknow";
        }
    }

    enum BENEFIT_TYPE {
        KEEPS_TYPE  = 0,
        REMAIN_TYPE = 1
    };

    myFinanceMainWindow *grandparent;
    Ui::myExchangeFormMoneyUp *ui;

    QButtonGroup *grpOperation;

    int caller; // 0 keeps change, 1 remain change;

    ///DATA
    QMap<int, int> exchangeIdx2AccountIdx;
    float remainAssetValue;
    float totalAssetValue;

    const myAccountNode *currentAccount;
    const myAssetNode   *currentAsset;

    int benefitIdx;
    float benefits;

    void updateBuySell();
};

#endif // MYEXCHANGEFORMMONEYUP_H
