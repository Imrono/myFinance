#include "myModifyExchange.h"
#include "ui_myModifyExchange.h"

myModifyExchange::myModifyExchange(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::myModifyExchange)
{
    ui->setupUi(this);
    isRollback = ui->checkBoxRollback->isChecked();
}

myModifyExchange::~myModifyExchange()
{
    delete ui;
}

void myModifyExchange::setUI(myExchangeData exchangeData) {
    ui->lineEditId->setText(QString("%1").arg(exchangeData.id));
    ui->dateTimeEditTime->setDateTime(exchangeData.time);
    ui->lineEditType->setText(exchangeData.type);

    ui->lineEditAccount1->setText(exchangeData.account1);
    ui->spinBoxMoney->setValue(exchangeData.money);

    ui->lineEditAccount2->setText(exchangeData.account2);
    ui->lineEditCode->setText(exchangeData.code);
    ui->lineEditName->setText(exchangeData.name);
    ui->spinBoxAmount->setValue(exchangeData.amount);
    ui->spinBoxPrice->setValue(exchangeData.price);
}

void myModifyExchange::on_lineEditCode_textChanged(const QString &str) {
    if (str == "cash") {
        ui->spinBoxAmount->setDisabled(true);
        ui->labelPrice->setText(QString::fromLocal8Bit("资金："));
    } else {
        if (!ui->spinBoxAmount->isEnabled()) {
            ui->spinBoxAmount->setEnabled(true);
            ui->labelPrice->setText(QString::fromLocal8Bit("单价："));
        }
    }
}

void myModifyExchange::on_buttonBox_accepted()
{
    data.id = ui->lineEditId->text().toInt();
    data.time = ui->dateTimeEditTime->dateTime();
    data.type = ui->lineEditType->text();

    data.account1 = ui->lineEditAccount1->text();
    data.money = ui->spinBoxMoney->value();

    data.account2 = ui->lineEditAccount2->text();
    data.code = ui->lineEditCode->text();
    data.name = ui->lineEditName->text();
    data.amount = ui->spinBoxAmount->value();
    data.price = ui->spinBoxPrice->value();
}

void myModifyExchange::on_checkBoxRollback_clicked()
{
    isRollback = ui->checkBoxRollback->isChecked();
}
