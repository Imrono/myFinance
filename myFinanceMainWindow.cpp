#include "myFinanceMainWindow.h"
#include "ui_myFinanceMainWindow.h"

#include "myDatabaseDatatype.h"

#include <QDebug>
#include <QMessageBox>
#include "myFinanceExchangeWindow.h"

myFinanceMainWindow::myFinanceMainWindow(myStockCodeName *inStockCode, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::myFinanceMainWindow), statusLabel(this),
    stockCode(inStockCode)
{
    assetModel = new myAssetModel(this);
    exchangeModel = new myExchangeListModel();

    ui->setupUi(this);

    ui->treeView->setModel(assetModel);
    //ui->treeView->header()->resizeSections(QHeaderView::ResizeToContents);
    ui->treeView->header()->resizeSections(QHeaderView::Fixed);
    ui->treeView->header()->resizeSection(0, 170);
    ui->treeView->header()->resizeSection(1, 90);
    ui->treeView->header()->resizeSection(2, 50);
    ui->treeView->expandAll();

    ui->listView->setModel(exchangeModel);

    connect(assetModel, SIGNAL(priceDataReflashed()), this, SLOT(priceDataReflashed()));
    connect(stockCode ,SIGNAL(codeDataReady()), this, SLOT(codeDataReady()));

    if (!stockCode->getIsDataReady()) { //���ڸ���
        ui->reflash->setEnabled(false);
        statusLabel.setText(QString::fromLocal8Bit("���ڶ�ȡ��Ʊ����"));
        ui->statusBar->addWidget(&statusLabel);
    } else {
        ui->reflash->setEnabled(true);
        statusLabel.setText("ready");
    }
}

myFinanceMainWindow::~myFinanceMainWindow()
{
    delete ui;
    delete assetModel;
    delete exchangeModel;
}
void myFinanceMainWindow::priceDataReflashed() {
    ui->treeView->header()->resizeSection(0, 170);
    ui->treeView->header()->resizeSection(1, 90);
    ui->treeView->header()->resizeSection(2, 50);

    ui->treeView->expandAll();
    ui->dateTimePrice->setDateTime(QDateTime::currentDateTime());
    qDebug() << QString::fromLocal8Bit("�۸���� finished");
}
void myFinanceMainWindow::codeDataReady() {
    if (stockCode->getIsDataReady()) {
        ui->reflash->setEnabled(true);
    }
    statusLabel.setText("ready");
}

void myFinanceMainWindow::on_exchange_clicked()
{
    qDebug() << QString::fromLocal8Bit("�ʲ��仯 clicked");
    if (!assetModel->getRootNode()) {
        QMessageBox::information(NULL, QString::fromLocal8Bit("��ʾ"), "rootNode is invalid");
        return;
    }
    myFinanceExchangeWindow exWin(this);
    if(exWin.exec() == QDialog::Accepted) {
        exchangeData data = exWin.getExchangeData();
        QString abnormalInfo;
        if (assetModel->checkExchange(data, abnormalInfo)) {
            assetModel->doExchange(data);
            exchangeModel->doExchange(data);
            qDebug() << QString::fromLocal8Bit("�ʲ��仯 Accepted assetModel->doExchange finished");
        } else {
            QMessageBox::information(NULL, QString::fromLocal8Bit("��ʾ"), abnormalInfo);
            qDebug() << QString::fromLocal8Bit("�ʲ��仯 Accepted assetModel->checkExchange failure");
        }
    } else {
        qDebug() << QString::fromLocal8Bit("�ʲ��仯 Canceled");
    }

    ui->treeView->expandAll();
}

void myFinanceMainWindow::on_new_account_clicked()
{
    qDebug() << QString::fromLocal8Bit("�½��ʻ� clicked");
    exchangeModel->test();
}

void myFinanceMainWindow::on_reflash_clicked()
{
    statusLabel.setText(QString::fromLocal8Bit("���ڶ�ȡ��Ʊ����"));
    ui->reflash->setEnabled(false);
    qDebug() << QString::fromLocal8Bit("ˢ�� clicked");
    assetModel->doReflashAssetData();
    stockCode->getStockCode();
    ui->treeView->expandAll();
    qDebug() << QString::fromLocal8Bit("ˢ�� clicked finished");
}

void myFinanceMainWindow::on_updatePrice_clicked()
{
    qDebug() << QString::fromLocal8Bit("���¼۸� clicked");
    assetModel->doUpdatePrice();
    ui->treeView->expandAll();
    qDebug() << QString::fromLocal8Bit("���¼۸� clicked assetModel->doUpdatePrice requested");
}
