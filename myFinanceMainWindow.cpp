#include "myFinanceMainWindow.h"
#include "ui_myFinanceMainWindow.h"

#include <QDebug>
#include <QMessageBox>
#include "myFinanceExchangeWindow.h"
#include "myInsertModifyAccount.h"
#include "myInsertModifyAsset.h"
#include "myModifyExchange.h"

myFinanceMainWindow::myFinanceMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::myFinanceMainWindow),
    assetModel(nullptr), exchangeModel(nullptr),
    stockCode(myStockCodeName::getInstance()), statusLabel(this),
    treeViewContextMenu(this)
{
    assetModel = new myAssetModel(this);
    //delegate = new assetChangeDelegate(this);
    exchangeModel = new myExchangeListModel();

    ui->setupUi(this);

    ui->treeView->setModel(assetModel);
    //ui->treeView->setItemDelegate(delegate);
    //ui->treeView->header()->resizeSections(QHeaderView::ResizeToContents);
    ui->treeView->header()->resizeSections(QHeaderView::Fixed);
    ui->treeView->header()->resizeSection(0, 180);
    ui->treeView->header()->resizeSection(1, 90);
    ui->treeView->header()->resizeSection(2, 50);
    ui->treeView->expandAll();

    ui->listView->setModel(exchangeModel);

    connect(assetModel, SIGNAL(priceDataReflashed()), this, SLOT(priceDataReflashed()));
    connect(stockCode ,SIGNAL(codeDataReady()), this, SLOT(codeDataReady()));

    ui->statusBar->addWidget(&statusLabel);
    if (!stockCode->getIsDataReady()) { //正在更新
        ui->reflash->setEnabled(false);
        statusLabel.setText(STR("正在读取股票代码"));
    } else {
        ui->reflash->setEnabled(true);
        statusLabel.setText("ready");
    }

    /// treeView
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeView, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(treeViewContextMenuSlot(const QPoint&)));

    /// listView
    editExchange = new QMenu(this);
    modifyExchange = new QAction(ui->listView);
    modifyExchange->setText(STR("更改交易"));
    deleteExchange = new QAction(ui->listView);
    deleteExchange->setText(STR("撤消交易"));
    editExchange->addAction(modifyExchange);
    editExchange->addAction(deleteExchange);

    connect(modifyExchange, SIGNAL(triggered()), this, SLOT(modifyExchange_clicked()));
    connect(deleteExchange, SIGNAL(triggered()), this, SLOT(deleteExchange_clicked()));

    ui->listView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->listView, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(listViewContextMenu(const QPoint&)));
}

myFinanceMainWindow::~myFinanceMainWindow()
{
    delete ui;
    if (nullptr != assetModel) {
        delete assetModel;
        assetModel = nullptr;
    }
//    if (nullptr != delegate) {
//        delete delegate;
//        delegate = nullptr;
//    }
    if (nullptr != exchangeModel) {
        delete exchangeModel;
        exchangeModel = nullptr;
    }
}

void myFinanceMainWindow::priceDataReflashed() {
    ui->treeView->header()->resizeSection(0, 180);
    ui->treeView->header()->resizeSection(1, 90);
    ui->treeView->header()->resizeSection(2, 50);

    ui->treeView->expandAll();
    ui->dateTimePrice->setDateTime(QDateTime::currentDateTime());
    float totalAsset = assetModel->doGetTotalAsset();
    QString strAsset = QString::number(totalAsset, 'f', 2);
    ui->lineEditTotalAsset->setText(strAsset);
    float securityAsset = assetModel->doGetSecurityAsset();
    strAsset = QString::number(securityAsset, 'f', 2);
    ui->lineEditSecurity->setText(strAsset);
    qDebug() << STR("价格更新 finished");
}
void myFinanceMainWindow::codeDataReady() {
    if (stockCode->getIsDataReady()) {
        ui->reflash->setEnabled(true);
    }
    statusLabel.setText("ready");
}

void myFinanceMainWindow::on_exchange_clicked()
{
    qDebug() << STR("资产变化 clicked");
    myExchangeUI exchangeUI = myExchangeUI();
    doExchange(exchangeUI);
}
void myFinanceMainWindow::doExchange(const myExchangeUI& uiSetup, bool isSetDisable) {
    myFinanceExchangeWindow exWin(this, uiSetup, isSetDisable);
    if(exWin.exec() == QDialog::Accepted) {
        myExchangeData data = exWin.getExchangeData();
        QString abnormalInfo;
        if (assetModel->checkExchange(data, abnormalInfo)) {
            assetModel->doExchange(data, true);
            exchangeModel->doExchange(data);
            qDebug() << STR("资产变化 Accepted assetModel->doExchange finished");
        } else {
            QMessageBox::information(NULL, STR("提示"), abnormalInfo);
            qDebug() << STR("资产变化 Accepted assetModel->checkExchange failure");
        }
    } else {
        qDebug() << STR("资产变化 Canceled");
    }

    ui->treeView->expandAll();
}

void myFinanceMainWindow::on_new_account_clicked()
{
    qDebug() << STR("新建帐户 clicked");
    myInsertModifyAccount dial(this);
    if(dial.exec() == QDialog::Accepted) {
        if(!assetModel->doInsertAccount(dial.getData())) {
            ui->treeView->expandAll();
            QMessageBox::warning(this, STR("新建帐户错误"),
                                 STR("新建帐户错误"),
                                 QMessageBox::Ok, QMessageBox::Ok);
        }
    } else {
        qDebug() << STR("添加帐户 Canceled");
    }
    ui->treeView->expandAll();
}

void myFinanceMainWindow::on_reflash_clicked()
{
    statusLabel.setText(STR("正在读取股票代码"));
    ui->reflash->setEnabled(false);
    qDebug() << STR("刷新 clicked");
    assetModel->doReflashData();
    stockCode->getStockCode();
    ui->treeView->expandAll();
    qDebug() << STR("刷新 clicked finished");
}

void myFinanceMainWindow::on_updatePrice_clicked()
{
    qDebug() << STR("更新价格 clicked");
    assetModel->doUpdatePrice();
    ui->treeView->expandAll();
    qDebug() << STR("更新价格 clicked assetModel->doUpdatePrice requested");
}

///////////////////////////////////////////////////////////////////////////////////////////
/// treeView的右键菜单
void myFinanceMainWindow::treeViewContextMenuSlot(const QPoint& pt) {
    QModelIndex index = ui->treeView->indexAt(pt);
    myAssetNode *node = assetModel->nodeFromIndex(index);
    treeViewContextMenu.treeViewContextMenu(node);
}
void myFinanceMainWindow::doChangeAssetDirectly(const myAssetNode *node, changeType type, QVariant data, const QString &info) {
    if (!assetModel->doChangeAssetDirectly(node, type, data)) {
        ui->treeView->expandAll();
        QMessageBox::warning(this, info+"failed", info+"failed", QMessageBox::Ok, QMessageBox::Ok);
    }
    ui->treeView->expandAll();
}

void myFinanceMainWindow::doUpDown(const myAssetNode *node, bool isUp) {
    if (!assetModel->doUpDown(isUp, node)) {
        ui->treeView->expandAll();
        QMessageBox::warning(this, "doUpDown failed", "doUpDown failed", QMessageBox::Ok, QMessageBox::Ok);
    }
    ui->treeView->expandAll();
}

///////////////////////////////////////////////////////////////////////////////////////////
/// listView的右键菜单
void myFinanceMainWindow::listViewContextMenu(const QPoint& pt) {
    QModelIndex index = ui->listView->indexAt(pt);
    if (!index.isValid()) {
        modifyExchange->setDisabled(true);
        deleteExchange->setDisabled(true);
    } else {
        modifyExchange->setEnabled(true);
        deleteExchange->setEnabled(true);
    }

    editExchange->exec(QCursor::pos());
}
void myFinanceMainWindow::modifyExchange_clicked() {
    bool ans = true;
    QString info = STR("更改资产变化");
    int line = ui->listView->currentIndex().row();
    if (line < 0) {
        return;
    }
    myExchangeData originExchangeData = exchangeModel->getDataFromRow(line);
    myExchangeUI tmpUI(originExchangeData);
    myFinanceExchangeWindow exWin(this, tmpUI);
    exWin.setWindowTitle(info);
    //exWin.setUI(originExchangeData, true);
    if(exWin.exec() == QDialog::Accepted) {
        qDebug() << info + "Accepted";
        // 1. DO EXCHANGE ASSET_DATA
        myExchangeData targetExchangeData = exWin.getExchangeData();
        myExchangeData doExchangeData = targetExchangeData;
        int type = myExchangeListModel::NO_DO_EXCHANGE;
        exchangeModel->coordinatorModifyExchange(originExchangeData, targetExchangeData, type);
        if (exWin.getIsRollback()) {
            ans = assetModel->doExchange(-originExchangeData) && ans;
            ans = assetModel->doExchange(targetExchangeData) && ans;
        }
        // 2. DO EXCHANGE EXCHANGE_DATA
        if (false == ans) {
            QMessageBox::warning(this, info, info + " ERROR", QMessageBox::Ok, QMessageBox::Ok);
        } else {
            if (myExchangeListModel::NO_DO_EXCHANGE == type) {
                qDebug() << "NO EXCHANGE DATA CHANGES";
            } else {
                exchangeModel->doExchange(doExchangeData);
            }
        }
    }
    ui->treeView->expandAll();
}
void myFinanceMainWindow::deleteExchange_clicked() {
    bool ans = true;
    QString info = STR("撤消资产变化");
    int line = ui->listView->currentIndex().row();
    if (line < 0) {
        return;
    }
    myExchangeData originExchangeData = exchangeModel->getDataFromRow(line);
    myExchangeUI tmpUI(originExchangeData);
    myFinanceExchangeWindow dial(this, tmpUI);
    dial.setWindowTitle(info);
    dial.setUI(originExchangeData);
    //dial.setUI4Delete();
    if(dial.exec() == QDialog::Accepted) {
        qDebug() << info + "Accepted";
        // 1. DO EXCHANGE ASSET_DATA
        if (dial.getIsRollback()) {
            ans = assetModel->doExchange(-originExchangeData) && ans;
        }
        // 2. DO EXCHANGE EXCHANGE_DATA
        if (false == ans) {
            QMessageBox::warning(this, info, info + " ERROR", QMessageBox::Ok, QMessageBox::Ok);
        } else {
            exchangeModel->doExchange(originExchangeData, true);
        }
    }
    ui->treeView->expandAll();
}

void myFinanceMainWindow::doDividend(const myDividends &divident, const myAssetData &nodeAssetData, myExchangeData &exchangeData) {
    assetModel->doDividend(divident, nodeAssetData, exchangeData);
}
