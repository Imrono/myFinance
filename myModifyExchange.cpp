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

    ui->lineEditAccount1->setText(data.account1);
    ui->spinBoxMoney->setValue(data.money);

    ui->lineEditAccount2->setText(data.account2);
    ui->lineEditCode->setText(data.code);
    ui->lineEditName->setText(data.name);
    ui->spinBoxAmount->setValue(data.amount);
    ui->spinBoxPrice->setValue(data.price);
}
void myModifyExchange::setUI4Delete() {
    ui->lineEditId->setDisabled(true);
    ui->dateTimeEditTime->setDisabled(true);
    ui->lineEditType->setDisabled(true);

    ui->lineEditAccount1->setDisabled(true);
    ui->spinBoxMoney->setDisabled(true);

    ui->lineEditAccount2->setDisabled(true);
    ui->lineEditCode->setDisabled(true);
    ui->lineEditName->setDisabled(true);
    ui->spinBoxAmount->setDisabled(true);
    ui->spinBoxPrice->setDisabled(true);
}

void myModifyExchange::on_lineEditCode_textChanged(const QString &str) {
    if (str == "cash") {
        data.amount = 1;
        ui->spinBoxAmount->setValue(data.amount);
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

    data.account1 = ui->lineEditAccount1->text();
    data.money = ui->spinBoxMoney->value();

    data.account2 = ui->lineEditAccount2->text();
    data.code = ui->lineEditCode->text();
    data.name = ui->lineEditName->text();
    data.amount = ui->spinBoxAmount->value();
    data.price = ui->spinBoxPrice->value();
}

myExchangeData myModifyExchange::getData() {
    return data;
}
bool myModifyExchange::isRollback() {
    return ui->checkBoxRollback->isChecked();
}

void myModifyExchange::on_spinBoxPrice_valueChanged(double price) {
    if (ui->lineEditCode->text() == MY_CASH) {
        data.price = price;
        data.money = - data.price;
        ui->spinBoxMoney->setValue(data.money);
    }
}

void myModifyExchange::on_spinBoxMoney_valueChanged(double money) {
    if (ui->lineEditCode->text() == MY_CASH) {
        data.money = money;
        data.price = - data.money;
        ui->spinBoxPrice->setValue(data.price);
    }
}
