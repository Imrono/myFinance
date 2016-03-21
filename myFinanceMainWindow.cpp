#include "myFinanceMainWindow.h"
#include "ui_myFinanceMainWindow.h"

#include "myDatabaseDatatype.h"

#include <QDebug>
#include <QMessageBox>
#include "myFinanceExchangeWindow.h"

myFinanceMainWindow::myFinanceMainWindow(myStockCodeName *inStockCode, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::myFinanceMainWindow), statusLabel(this),
    stockCode(inStockCode),
    assetModel(nullptr), exchangeModel(nullptr), changeAsset(nullptr),
    deleteAsset(nullptr), insertAsset(nullptr), modifyAsset(nullptr)
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

    if (!stockCode->getIsDataReady()) { //正在更新
        ui->reflash->setEnabled(false);
        statusLabel.setText(QString::fromLocal8Bit("正在读取股票代码"));
        ui->statusBar->addWidget(&statusLabel);
    } else {
        ui->reflash->setEnabled(true);
        statusLabel.setText("ready");
    }

    changeAsset = new QMenu(this);
    deleteAsset = new QAction(ui->treeView);
    insertAsset = new QAction(ui->treeView);
    modifyAsset = new QAction(ui->treeView);
    connect(deleteAsset, SIGNAL(triggered()), this, SLOT(deleteAsset_clicked()));
    connect(insertAsset, SIGNAL(triggered()), this, SLOT(insertAsset_clicked()));
    connect(modifyAsset, SIGNAL(triggered()), this, SLOT(modifyAsset_clicked()));

    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeView, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(treeViewContextMenu(const QPoint&)));
}

myFinanceMainWindow::~myFinanceMainWindow()
{
    delete ui;
    if (nullptr != assetModel) {
        delete assetModel;
        assetModel = nullptr;
    }
    if (nullptr != exchangeModel) {
        delete exchangeModel;
        exchangeModel = nullptr;
    }
    if (nullptr != deleteAsset) {
        delete deleteAsset;
        deleteAsset = nullptr;
    }
    if (nullptr != insertAsset) {
        delete insertAsset;
        insertAsset = nullptr;
    }
    if (nullptr != modifyAsset) {
        delete modifyAsset;
        modifyAsset = nullptr;
    }
    if (nullptr != changeAsset) {
        delete changeAsset;
        changeAsset = nullptr;
    }
}

void myFinanceMainWindow::priceDataReflashed() {
    ui->treeView->header()->resizeSection(0, 170);
    ui->treeView->header()->resizeSection(1, 90);
    ui->treeView->header()->resizeSection(2, 50);

    ui->treeView->expandAll();
    ui->dateTimePrice->setDateTime(QDateTime::currentDateTime());
    qDebug() << QString::fromLocal8Bit("价格更新 finished");
}
void myFinanceMainWindow::codeDataReady() {
    if (stockCode->getIsDataReady()) {
        ui->reflash->setEnabled(true);
    }
    statusLabel.setText("ready");
}

void myFinanceMainWindow::on_exchange_clicked()
{
    qDebug() << QString::fromLocal8Bit("资产变化 clicked");
    if (!assetModel->getRootNode()) {
        QMessageBox::information(NULL, QString::fromLocal8Bit("提示"), "rootNode is invalid");
        return;
    }
    myFinanceExchangeWindow exWin(this);
    if(exWin.exec() == QDialog::Accepted) {
        exchangeData data = exWin.getExchangeData();
        QString abnormalInfo;
        if (assetModel->checkExchange(data, abnormalInfo)) {
            assetModel->doExchange(data);
            exchangeModel->doExchange(data);
            qDebug() << QString::fromLocal8Bit("资产变化 Accepted assetModel->doExchange finished");
        } else {
            QMessageBox::information(NULL, QString::fromLocal8Bit("提示"), abnormalInfo);
            qDebug() << QString::fromLocal8Bit("资产变化 Accepted assetModel->checkExchange failure");
        }
    } else {
        qDebug() << QString::fromLocal8Bit("资产变化 Canceled");
    }

    ui->treeView->expandAll();
}

void myFinanceMainWindow::on_new_account_clicked()
{
    qDebug() << QString::fromLocal8Bit("新建帐户 clicked");
    exchangeModel->test();
}

void myFinanceMainWindow::on_reflash_clicked()
{
    statusLabel.setText(QString::fromLocal8Bit("正在读取股票代码"));
    ui->reflash->setEnabled(false);
    qDebug() << QString::fromLocal8Bit("刷新 clicked");
    assetModel->doReflashAssetData();
    stockCode->getStockCode();
    ui->treeView->expandAll();
    qDebug() << QString::fromLocal8Bit("刷新 clicked finished");
}

void myFinanceMainWindow::on_updatePrice_clicked()
{
    qDebug() << QString::fromLocal8Bit("更新价格 clicked");
    assetModel->doUpdatePrice();
    ui->treeView->expandAll();
    qDebug() << QString::fromLocal8Bit("更新价格 clicked assetModel->doUpdatePrice requested");
}

void myFinanceMainWindow::contextMenuEvent(QContextMenuEvent *event) {
    //changeAsset->exec(QCursor::pos());
    event->accept();
}
void myFinanceMainWindow::treeViewContextMenu(const QPoint& pt) {
    myAssetNode *node = assetModel->nodeFromIndex(ui->treeView->currentIndex());
    changeAsset->clear();
    if (myAssetNode::nodeAccount == node->type) {
        insertAsset->setText(QString::fromLocal8Bit("添加资产"));
        changeAsset->addAction(insertAsset);
        modifyAsset->setText(QString::fromLocal8Bit("更新帐户"));
        changeAsset->addAction(modifyAsset);
        deleteAsset->setText(QString::fromLocal8Bit("删除帐户"));
        changeAsset->addAction(deleteAsset);
    } else if (myAssetNode::nodeHolds == node->type) {
        modifyAsset->setText(QString::fromLocal8Bit("更新资产"));
        changeAsset->addAction(modifyAsset);
        deleteAsset->setText(QString::fromLocal8Bit("删除资产"));
        changeAsset->addAction(deleteAsset);
    } else if (myAssetNode::nodeRoot == node->type) {
    } else {}

    changeAsset->exec(QCursor::pos());
}
void myFinanceMainWindow::deleteAsset_clicked() {
    doChangeAssetDirectly(POP_DELETE);
}
void myFinanceMainWindow::insertAsset_clicked() {
    doChangeAssetDirectly(POP_INSERT);
}
void myFinanceMainWindow::modifyAsset_clicked() {
    doChangeAssetDirectly(POP_MODIFY);
}
void myFinanceMainWindow::doChangeAssetDirectly(changeType type) {
    myAssetNode *node = assetModel->nodeFromIndex(ui->treeView->currentIndex());

    if (myAssetNode::nodeAccount == node->type) {
        if (POP_INSERT == type) {
            QDialog dial;
            if(dial.exec() == QDialog::Accepted) {

            } else { return; }
        } else if (POP_MODIFY == type) {

        } else if (POP_DELETE == type) {
            if(QMessageBox::Ok != QMessageBox::warning(this, QString::fromLocal8Bit("删除帐户"),
                                  QString::fromLocal8Bit("删除帐户：\n") +
                                  node->nodeData.value<myAssetAccount>().code + "\n" + node->nodeData.value<myAssetAccount>().name,
                                  QMessageBox::Ok|QMessageBox::Cancel, QMessageBox::Ok)) {
                return;
            }
        } else {}
    } else if (myAssetNode::nodeHolds == node->type) {
        if (POP_MODIFY == type) {
            QDialog dial;
            if(dial.exec() == QDialog::Accepted) {

            } else { return; }
        } else if (POP_DELETE == type) {
            if(QMessageBox::Ok != QMessageBox::warning(this, QString::fromLocal8Bit("删除资产"),
                                  QString::fromLocal8Bit("删除资产：\n") +
                                  node->nodeData.value<myAssetHold>().assetCode + "\n" + node->nodeData.value<myAssetHold>().name,
                                  QMessageBox::Ok|QMessageBox::Cancel, QMessageBox::Ok)) {
                return;
            }
        } else {}
    } else if (myAssetNode::nodeRoot == node->type) {
    } else {}
    qDebug() << "view" << (int)type;
    assetModel->doChangeAssetDirectly(node, type);
    ui->treeView->expandAll();
}
