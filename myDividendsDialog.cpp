#include "myDividendsDialog.h"
#include "ui_myDividendsDialog.h"

myDividendsDialog::myDividendsDialog(const myAssetData &assetData, bool isIntrest, QWidget *parent) :
    QDialog(parent), isIntrest(isIntrest), assetData(assetData),
    ui(new Ui::myDividendsDialog)
{
    ui->setupUi(this);
    ui->accountLineEdit->setText(assetData.accountCode);
    ui->codeLineEdit->setText(assetData.assetCode);
    ui->nameLineEdit->setText(assetData.assetName);
    ui->accountLineEdit->setDisabled(true);
    ui->codeLineEdit->setDisabled(true);
    ui->nameLineEdit->setDisabled(true);

    if (isIntrest) {
        ui->currentHoldCheckBox->setHidden(false);
        setPatternIntrest();
        dividendsData.type = myDividends::INTRESTS;
    } else {
        ui->currentHoldCheckBox->setHidden(true);
        dividendsData.type = myDividends::STOCK_DIVIDEND;
        ui->baseSpinBox->setValue(10);
    }
}

myDividendsDialog::~myDividendsDialog()
{
    delete ui;
}

void myDividendsDialog::on_baseSpinBox_valueChanged(int value) {
    dividendsData.base = value;
}

void myDividendsDialog::on_shareSplitSpinBox_valueChanged(double value) {
    dividendsData.shareSplit = value;
}

void myDividendsDialog::on_shareBonusSpinBox_valueChanged(double value) {
    dividendsData.shareBonus = value;
}

void myDividendsDialog::on_capitalBonusSpinBox_valueChanged(double value) {
    if (isIntrest && ui->currentHoldCheckBox->isChecked()) {
        dividendsData.capitalBonus = value - assetData.price;
    } else {
        dividendsData.capitalBonus = value;
    }
}

void myDividendsDialog::on_dateTimeEdit_dateTimeChanged(const QDateTime &dateTime) {
    dividendsData.time = dateTime;
}

void myDividendsDialog::setPatternIntrest() {
    ui->baseSpinBox->setValue(1);
    ui->baseSpinBox->setDisabled(true);
    ui->shareSplitSpinBox->setValue(0.0f);
    ui->shareSplitSpinBox->setDisabled(true);
    ui->shareBonusSpinBox->setValue(0.0f);
    ui->shareBonusSpinBox->setDisabled(true);
}
