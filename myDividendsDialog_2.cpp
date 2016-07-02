#include "myDividendsDialog_2.h"
#include "ui_myDividendsDialog_2.h"

myDividendsDialog_2::myDividendsDialog_2(const myAssetData &assetHold, QWidget *parent) :
    QDialog(parent), assetHold(assetHold), exchangeMoney(0.0f),
    totalKeeps(0.0f), remains(0.0f), dividents(0.0f),
    ui(new Ui::myDividendsDialog_2)
{
    ui->setupUi(this);

    ui->originalTotalSpinBox->setValue(assetHold.price);
    ui->dividentTotalSpinBox->setValue(assetHold.price);
}

myDividendsDialog_2::~myDividendsDialog_2()
{
    delete ui;
}

void myDividendsDialog_2::on_moneySpinBox_valueChanged(double value) {
    exchangeMoney = value;
    ui->originalRemainSpinBox->setValue(assetHold.price - exchangeMoney);
    ui->dividentRemainSpinBox->setValue(totalKeeps - exchangeMoney);
}

void myDividendsDialog_2::on_dividentTotalSpinBox_valueChanged(double value) {
    totalKeeps = value;
    dividents = totalKeeps - assetHold.price;

    ui->dividentRemainSpinBox->setValue(totalKeeps - exchangeMoney);
}

void myDividendsDialog_2::on_dividentRemainSpinBox_valueChanged(double value) {
    remains = value;

    ui->dividentTotalSpinBox->setValue(remains + exchangeMoney);
}

void myDividendsDialog_2::on_originalTotalSpinBox_valueChanged(double value) {
    ui->originalRemainSpinBox->setValue(value - exchangeMoney);
}
