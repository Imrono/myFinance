#ifndef MYEXCHANGEFORMFUND_H
#define MYEXCHANGEFORMFUND_H

#include <QWidget>
#include <QButtonGroup>
#include "myExchangeFormTabBase.h"

namespace Ui {
class myExchangeFormFund;
}

class myExchangeFormFund : public myExchangeFormTabBase
{
    Q_OBJECT

public:
    explicit myExchangeFormFund(const myAccountAssetRootNode *rootNode, QString tabName, QWidget *parent = 0, bool isModifyExchange = false);
    ~myExchangeFormFund();

    void exchangeWindowFeeChanged(double fee);
    void recordExchangeData(myExchangeData &tmpData);
    void setUI(const myExchangeData &exchangeData);
    void checkAndSetDisable(const myExchangeData &exchangeData);

private slots:
    void on_radioSubscribing_clicked();
    void on_radioRedeming_clicked();

    void on_netValueSpinBox_valueChanged(double value);

    void on_keepsSpinBox_valueChanged(double value);
    void on_usedSpinBox_valueChanged(double value);
    void on_remainSpinBox_valueChanged(double value);

    void on_moneySpinBoxTotal_valueChanged(double value);
    void on_moneySpinBox_valueChanged(double value);
    void on_moneySpinBoxRemain_valueChanged(double value);

    void on_moneyAccount_currentIndexChanged(int index);
    void on_codeLineEdit_textEdited(const QString &str);
    void on_nameLineEdit_textChanged(const QString &str);

    void on_typeBox_currentIndexChanged(int index);

    void on_checkBoxSoldAll_clicked();

private:
    enum OperationType {
        SUBSCRIBING = 0,
        //PURCHASING  = 1,
        REDEMING    = 1
    };
    Ui::myExchangeFormFund *ui;
    QButtonGroup *grpOperation;

    ///DATA
    QMap<int, int> exchangeIdx2AccountIdx;

    float remainAssetValue;
    float totalAssetValue;

    const myAccountNode *currentAccount;
    const myAssetNode   *currentAsset;

    float getAssetCodeValue(const QString &code);
    QString getCurrentAssetName() {
        QString name;
        if (currentAsset) {
            name = GET_CONST_ASSET_NODE_DATA(currentAsset).assetData.assetName;
        }
        return name;
    }

    void updateBuySell();
};

#endif // MYEXCHANGEFORMFUND_H
