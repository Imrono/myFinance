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
    data.Code = ui->lineEditCode->text();
    data.Name = ui->lineEditName->text();
    data.Type = ui->lineEditType->text();
    data.Note = ui->lineEditNote->text();
}

void myInsertModifyAccount::setUI(myAccountData accountData) {
    ui->lineEditCode->setText(accountData.Code);
    ui->lineEditName->setText(accountData.Name);
    ui->lineEditType->setText(accountData.Type);
    ui->lineEditNote->setText(accountData.Note);
    data.originCode = accountData.Code;
}
