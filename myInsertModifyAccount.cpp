#include "myInsertModifyAccount.h"
#include "ui_myInsertModifyAccount.h"

myInsertModifyAccount::myInsertModifyAccount(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::myInsertAccount)
{
    ui->setupUi(this);
}

myInsertModifyAccount::~myInsertModifyAccount()
{
    delete ui;
}

void myInsertModifyAccount::on_buttonBox_accepted()
{
    data.code = ui->lineEditCode->text();
    data.name = ui->lineEditName->text();
    data.type = ui->lineEditType->text();
    data.note = ui->lineEditNote->text();
}

void myInsertModifyAccount::setUI(myAccountData accountData) {
    ui->lineEditCode->setText(accountData.code);
    ui->lineEditName->setText(accountData.name);
    ui->lineEditType->setText(accountData.type);
    ui->lineEditNote->setText(accountData.note);
}
