#include "myModifyExchange.h"
#include "ui_myModifyExchange.h"

myModifyExchange::myModifyExchange(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::myModifyExchange)
{
    ui->setupUi(this);
}

myModifyExchange::~myModifyExchange()
{
    delete ui;
}

void myModifyExchange::setUI(const myExchangeData &exchangeData) {
    data = exchangeData;
    ui->lineEditId->setText(QString("%1").arg(data.id));
    ui->dateTimeEditTime->setDateTime(data.time);
    ui->lineEditType->setText(data.exchangeType);

    ui->lineEditaccountMoney->setText(data.accountMoney);
    ui->spinBoxMoney->setValue(data.money);

    ui->lineEditAccount2->setText(data.assetData.accountCode);
    ui->lineEditCode->setText(data.assetData.assetCode);
    ui->lineEditName->setText(data.assetData.assetName);
    ui->spinBoxAmount->setValue(data.assetData.amount);
    ui->spinBoxPrice->setValue(data.assetData.price);
}
void myModifyExchange::setUI4Delete() {
    ui->lineEditId->setDisabled(true);
    ui->dateTimeEditTime->setDisabled(true);
    ui->lineEditType->setDisabled(true);

    ui->lineEditaccountMoney->setDisabled(true);
    ui->spinBoxMoney->setDisabled(true);

    ui->lineEditAccount2->setDisabled(true);
    ui->lineEditCode->setDisabled(true);
    ui->lineEditName->setDisabled(true);
    ui->spinBoxAmount->setDisabled(true);
    ui->spinBoxPrice->setDisabled(true);
}

void myModifyExchange::on_lineEditCode_textChanged(const QString &str) {
    if (str == "cash") {
        data.assetData.amount = 1;
        ui->spinBoxAmount->setValue(data.assetData.amount);
        ui->spinBoxAmount->setDisabled(true);
        ui->labelPrice->setText(STR("金额"));
    } else {
        if (!ui->spinBoxAmount->isEnabled()) {
            ui->spinBoxAmount->setEnabled(true);
            ui->labelPrice->setText(STR("单价"));
        }
    }
}

void myModifyExchange::on_buttonBox_accepted()
{
    data.id = ui->lineEditId->text().toInt();
    data.time = ui->dateTimeEditTime->dateTime();
    data.exchangeType = ui->lineEditType->text();

    data.accountMoney = ui->lineEditaccountMoney->text();
    data.money = ui->spinBoxMoney->value();

    data.assetData.accountCode = ui->lineEditAccount2->text();
    data.assetData.assetCode   = ui->lineEditCode->text();
    data.assetData.assetName   = ui->lineEditName->text();
    data.assetData.amount      = ui->spinBoxAmount->value();
    data.assetData.price       = ui->spinBoxPrice->value();
}

myExchangeData myModifyExchange::getData() {
    return data;
}
bool myModifyExchange::isRollback() {
    return ui->checkBoxRollback->isChecked();
}

void myModifyExchange::on_spinBoxPrice_valueChanged(double price) {
    if (ui->lineEditCode->text() == MY_CASH) {
        data.assetData.price = price;
        data.money = - data.assetData.price;
        ui->spinBoxMoney->setValue(data.money);
    }
}

void myModifyExchange::on_spinBoxMoney_valueChanged(double money) {
    if (ui->lineEditCode->text() == MY_CASH) {
        data.money = money;
        data.assetData.price = - data.money;
        ui->spinBoxPrice->setValue(data.assetData.price);
    }
}
