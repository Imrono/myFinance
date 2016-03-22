#include "myInsertAccount.h"
#include "ui_myInsertAccount.h"

myInsertAccount::myInsertAccount(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::myInsertAccount)
{
    ui->setupUi(this);
}

myInsertAccount::~myInsertAccount()
{
    delete ui;
}

void myInsertAccount::on_buttonBox_accepted()
{
    data.Code = ui->lineEditCode->text();
    data.Name = ui->lineEditName->text();
    data.Type = ui->lineEditType->text();
    data.Note = ui->lineEditNote->text();
}
