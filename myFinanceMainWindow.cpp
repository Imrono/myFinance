#include "myFinanceMainWindow.h"
#include "ui_myFinanceMainWindow.h"

#include "myDatabaseDatatype.h"

#include <QMessageBox>
#include "myFinanceExchangeWindow.h"

myFinanceMainWindow::myFinanceMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::myFinanceMainWindow)
{
    assetModel = new myAssetModel(this);
    ui->setupUi(this);

    ui->treeView->setModel(assetModel);
    ui->treeView->header()->resizeSections(QHeaderView::ResizeToContents);
    ui->treeView->expandAll();
}

myFinanceMainWindow::~myFinanceMainWindow()
{
    delete ui;
    delete assetModel;
}

void myFinanceMainWindow::on_exchange_clicked()
{
    if (!assetModel->getRootNode()) {
        QMessageBox::information(NULL, "提示", "rootNode NULL");
        return;
    }
    myFinanceExchangeWindow exWin(assetModel->getRootNode(), this);
    if(exWin.exec() == QDialog::Accepted) {
        //QMessageBox::information(NULL, "提示", "OK");
        assetModel->doExchange(exWin.getExchangeData());
    } else {
        QMessageBox::information(NULL, "提示", "Cancel");
    }

    ui->treeView->expandAll();
}

void myFinanceMainWindow::on_new_account_clicked()
{
    QMessageBox::information(this,"提示","这是一个消息框 new_account");
}

void myFinanceMainWindow::on_reflash_clicked()
{
    assetModel->doReflash();
    ui->treeView->expandAll();
}
