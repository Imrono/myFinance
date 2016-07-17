#ifndef MYDIVIDENDSDIALOG_2_H
#define MYDIVIDENDSDIALOG_2_H
#include "myGlobal.h"

#include <QDialog>
#include "myDatabaseDatatype.h"

namespace Ui {
class myDividendsDialog_2;
}

class myDividendsDialog_2 : public QDialog
{
    Q_OBJECT

public:
    explicit myDividendsDialog_2(const myAssetData &assetHold, QWidget *parent = 0);
    ~myDividendsDialog_2();

    const myDividends &getDividentData() {
        dividentData.base = 1;
        dividentData.type = myDividends::INTRESTS;
        dividentData.capitalBonus = dividents;
        return dividentData;
    }

private slots:
    void on_moneySpinBox_valueChanged(double value);

    void on_dividentTotalSpinBox_valueChanged(double value);

    void on_dividentRemainSpinBox_valueChanged(double value);

    void on_originalTotalSpinBox_valueChanged(double value);

private:
    Ui::myDividendsDialog_2 *ui;
    const myAssetData &assetHold;

    float totalKeeps;
    float remains;
    float dividents;
    float exchangeMoney;

    myDividends dividentData;
};

#endif // MYDIVIDENDSDIALOG_2_H
