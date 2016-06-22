#ifndef DIVIDENDSDIALOG_H
#define DIVIDENDSDIALOG_H

#include <QDialog>
#include "myDatabaseDatatype.h"

namespace Ui {
class myDividendsDialog;
}

class myDividendsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit myDividendsDialog(const myAssetData &assetData, bool isIntrest, QWidget *parent);
    ~myDividendsDialog();

    myDividends getDividendsData() {
        return dividendsData;
    }
    void setPatternIntrest();

private slots:
    void on_baseSpinBox_valueChanged(int value);

    void on_shareSplitSpinBox_valueChanged(double value);

    void on_shareBonusSpinBox_valueChanged(double value);

    void on_capitalBonusSpinBox_valueChanged(double value);

    void on_dateTimeEdit_dateTimeChanged(const QDateTime &dateTime);

private:
    Ui::myDividendsDialog *ui;
    myDividends dividendsData;
    bool isIntrest;
    myAssetData assetData;
};

#endif // DIVIDENDSDIALOG_H
