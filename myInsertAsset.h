#ifndef MYINSERTASSET_H
#define MYINSERTASSET_H

#include <QDialog>
#include <QButtonGroup>

#include "myStockCodeName.h"
#include "myDatabaseDatatype.h"

namespace Ui {
class myInsertAsset;
}

class myInsertAsset : public QDialog
{
    Q_OBJECT

public:
    explicit myInsertAsset(QString accountCode, QString accountName, QWidget *parent = 0);
    ~myInsertAsset();

    insertAssetData getData() {
        return data;
    }

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


    insertAssetData data;
};

#endif // MYINSERTASSET_H
