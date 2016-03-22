#include "myInsertAsset.h"
#include "ui_myInsertAsset.h"

#include "myFinanceMainWindow.h"

#include <QDebug>

myInsertAsset::myInsertAsset(QString accountCode, QString accountName, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::myInsertAsset),
    stockCode(static_cast<myFinanceMainWindow *>(parent)->getStockCode())
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
}

myInsertAsset::~myInsertAsset()
{
    if (!grpMarket)
        delete grpMarket;

    delete ui;
}

void myInsertAsset::on_buttonBox_accepted()
{
    data.assetCode = ui->lineEditAssetCode->text();
    data.assetName = ui->lineEditAssetName->text();
    data.amount = ui->lineEditAmount->text().toInt();
    data.price  = ui->lineEditPrice->text().toDouble();
    data.type   = ui->lineEditType->text();
}

void myInsertAsset::updateMarketInfo() {
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
    ui->lineEditAssetCode->setText(data.assetCode);


    qDebug() << "updateMarketInfo()" << ","
             << "data.code"  << data.assetCode << ","
             << (grpMarket->checkedId() == SH ? "radioSH" :
                 grpMarket->checkedId() == SZ ? "radioSZ" :
                 grpMarket->checkedId() == OTHER ? "radioOther" : "radioUnknown");
}


void myInsertAsset::on_radioSH_clicked() {
    qDebug() << "radioSH clicked";
    updateMarketInfo();
}

void myInsertAsset::on_radioSZ_clicked() {
    qDebug() << "radioSZ clicked";
    updateMarketInfo();
}

void myInsertAsset::on_radioOther_clicked() {
    qDebug() << "radioOther clicked";
    updateMarketInfo();
}

void myInsertAsset::on_lineEditAssetCode_textChanged(const QString &str)
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
        } else {}
    }

    updateMarketInfo();
}

void myInsertAsset::on_lineEditAssetCode_editingFinished()
{
    int count = stockCode->codeName.count();
    qDebug() << "代号EditLine" << ui->lineEditAssetCode->text() << "(" << count << ")";
    if (OTHER != grpMarket->checkedId()) {
        if (stockCode->getIsInitialed()) {
            if (stockCode->codeName.contains(data.assetCode)) {
                QMap<QString, QString>::const_iterator ii = stockCode->codeName.find(data.assetCode);
                if (ii != stockCode->codeName.end() && ii.key() == data.assetCode) {
                    data.assetName = ii.value();
                    ui->lineEditAssetName->setText(data.assetName);
                }
            } else {
                data.assetName = "";
                ui->lineEditAssetName->setText(data.assetName);
            }
        }
    }
}

void myInsertAsset::on_lineEditAssetName_editingFinished()
{
    QString str = ui->lineEditAssetName->text();
    int count = stockCode->codeName.count();
    qDebug() << QString::fromLocal8Bit("名称EditLine") << str << "(" << count << ")";

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
