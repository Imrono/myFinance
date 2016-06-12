#include "myInsertModifyAsset.h"
#include "ui_myInsertModifyAsset.h"

#include "myFinanceMainWindow.h"

#include <QDebug>

myInsertModifyAsset::myInsertModifyAsset(QString accountCode, QString accountName, QWidget *parent, bool setAllDisable) :
    QDialog(parent),
    ui(new Ui::myInsertAsset),
    stockCode(myStockCodeName::getInstance())
{
    data.accountCode = accountCode;

    ui->setupUi(this);
    ui->labelAccount->setText(accountCode + " - " + accountName);

    grpMarket = new QButtonGroup(this);
    grpMarket->addButton(ui->radioSH);
    grpMarket->addButton(ui->radioSZ);
    grpMarket->addButton(ui->radioOther);
    grpMarket->setExclusive(true);           //设为互斥
    grpMarket->setId(ui->radioSH, SH);       //radioBuy的Id设为0
    grpMarket->setId(ui->radioSZ, SZ);       //radioBuy的Id设为1
    grpMarket->setId(ui->radioOther, OTHER); //radioBuy的Id设为2
    ui->radioSH->setChecked(true);

    if (setAllDisable)
        setAllDisabled();
}

myInsertModifyAsset::~myInsertModifyAsset()
{
    if (!grpMarket)
        delete grpMarket;

    delete ui;
}

void myInsertModifyAsset::on_buttonBox_accepted()
{
    data.assetCode = ui->lineEditAssetCode->text();
    data.assetName = ui->lineEditAssetName->text();
    data.amount    = ui->spinBoxAmount->value();
    data.price     = ui->spinBoxPrice->value();
    data.type      = ui->lineEditType->text();
}

void myInsertModifyAsset::updateMarketInfo() {
    QString market;
    switch (grpMarket->checkedId()) {
    case SH:
        market = "sh.";
        break;
    case SZ:
        market = "sz.";
        break;
    case OTHER:
        market = "";
        break;
    default:
        break;
    }
    int pointIndex = data.assetCode.indexOf(QString("."));
    data.assetCode.remove(0, pointIndex+1);
    data.assetCode.insert(0, market);
    if (data.assetCode == "sh.sh") {
        data.assetCode = "sh.";
    } else if (data.assetCode == "sz.sz") {
        data.assetCode = "sz.";
    } else {}
    ui->lineEditAssetCode->setText(data.assetCode);


    qDebug() << "updateMarketInfo()" << ","
             << "data.code"  << data.assetCode << ","
             << (grpMarket->checkedId() == SH ? "radioSH" :
                 grpMarket->checkedId() == SZ ? "radioSZ" :
                 grpMarket->checkedId() == OTHER ? "radioOther" : "radioUnknown");
}


void myInsertModifyAsset::on_radioSH_clicked() {
    qDebug() << "radioSH clicked";
    updateMarketInfo();
}

void myInsertModifyAsset::on_radioSZ_clicked() {
    qDebug() << "radioSZ clicked";
    updateMarketInfo();
}

void myInsertModifyAsset::on_radioOther_clicked() {
    qDebug() << "radioOther clicked";
    updateMarketInfo();
}

void myInsertModifyAsset::on_lineEditAssetCode_textChanged(const QString &str)
{
    data.assetCode = str;

    // 上海，深圳通过股票代码自动判断
    int pointIndex = data.assetCode.indexOf(QString("."));
    int len = data.assetCode.size();
    if (len - pointIndex > 2 &&
        (grpMarket->checkedId() == SH ||
         grpMarket->checkedId() == SZ)) {
        QString subStr = data.assetCode.mid(pointIndex+1, 2);
        if (subStr == "30" || subStr == "00") {
            ui->radioSZ->setChecked(true);
        } else if (subStr == "60") {
            ui->radioSH->setChecked(true);
        } else {
            subStr = data.assetCode.mid(pointIndex+1, 4);
            if (subStr == "cash") {
                ui->radioOther->setChecked(true);
            } else {}
        }
    }

    updateMarketInfo();

    if (data.assetCode == "cash") {
        data.amount = 1;
        ui->spinBoxAmount->setValue(data.amount);
        ui->spinBoxAmount->setDisabled(true);
        ui->labelPrice->setText(STR("资金："));
    } else {
        if (!ui->spinBoxAmount->isEnabled()) {
            ui->spinBoxAmount->setEnabled(true);
            ui->labelPrice->setText(STR("单价："));
        }
    }
}

void myInsertModifyAsset::on_lineEditAssetCode_editingFinished()
{
    int count = stockCode->codeName.count();
    qDebug() << STR("代号EditLine") << ui->lineEditAssetCode->text() << "(" << count << ")";
    if (OTHER != grpMarket->checkedId()) {
        if (stockCode->getIsInitialed()) {
            data.assetName = stockCode->findNameFromCode(data.assetCode);
            ui->lineEditAssetName->setText(data.assetName);
        }
    }
}

void myInsertModifyAsset::on_lineEditAssetName_editingFinished()
{
    QString str = ui->lineEditAssetName->text();
    int count = stockCode->codeName.count();
    qDebug() << STR("名称EditLine") << str << "(" << count << ")";

    QMap<QString,QString>::iterator it = stockCode->codeName.begin();
    for (; it != stockCode->codeName.end(); ++it) {
        if (it.value() == str) {
            data.assetCode = it.key();
            QRegExp rx("([a-zA-Z]*)[.][0-9]*");
            int pos = data.assetCode.indexOf(rx);
            if (pos >= 0) {
                QString a = rx.cap(1);
                if (a == "sh") {
                    ui->radioSH->setChecked(true);
                } else if (a == "sz") {
                    ui->radioSZ->setChecked(true);
                } else {
                    ui->radioOther->setChecked(true);
                }
                updateMarketInfo();
            }

            ui->lineEditAssetCode->setText(data.assetCode);
            qDebug() << data.assetCode << it.value();
            return;
        }
    }

    ui->radioOther->setChecked(true);
    updateMarketInfo();
}

void myInsertModifyAsset::setUI(const myAssetData &assetData) {
    if (assetData.assetCode.left(3) == "sh.") {
        ui->radioSH->setChecked(true);
    } else if (assetData.assetCode.left(3) == "sz.") {
        ui->radioSZ->setChecked(true);
    } else {
        ui->radioOther->setChecked(true);
    }

    ui->lineEditAssetCode->setText(assetData.assetCode);
    ui->lineEditAssetName->setText(assetData.assetName);
    ui->spinBoxAmount->setValue(assetData.amount);
    ui->spinBoxPrice->setValue(assetData.price);
    ui->lineEditType->setText(assetData.type);
}

void myInsertModifyAsset::setAllDisabled() {
    ui->lineEditAssetCode->setDisabled(true);
    ui->lineEditAssetName->setDisabled(true);
    ui->spinBoxAmount->setDisabled(true);
    ui->spinBoxPrice->setDisabled(true);
    ui->lineEditType->setDisabled(true);

    ui->radioSH->setDisabled(true);
    ui->radioSZ->setDisabled(true);
    ui->radioOther->setDisabled(true);
}
