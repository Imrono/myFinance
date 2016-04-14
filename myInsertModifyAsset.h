#ifndef MYINSERTASSET_H
#define MYINSERTASSET_H
#include "myGlobal.h"

#include <QDialog>
#include <QButtonGroup>

#include "myStockCodeName.h"
#include "myDatabaseDatatype.h"

namespace Ui {
class myInsertAsset;
}

class myInsertModifyAsset : public QDialog
{
    Q_OBJECT

public:
    explicit myInsertModifyAsset(QString accountCode, QString accountName, QWidget *parent = 0);
    ~myInsertModifyAsset();

    myAssetData getData() {
        return data;
    }
    void setUI(myAssetData assetData);

private slots:
    void on_buttonBox_accepted();

    void on_radioSZ_clicked();

    void on_radioSH_clicked();

    void on_radioOther_clicked();

    void on_lineEditAssetCode_textChanged(const QString &str);

    void on_lineEditAssetCode_editingFinished();

    void on_lineEditAssetName_editingFinished();

private:
    Ui::myInsertAsset *ui;
    QButtonGroup *grpMarket;
    void updateMarketInfo();
    myStockCodeName *stockCode; //用于从code到name的推导

    myAssetData data;
};

#endif // MYINSERTASSET_H
