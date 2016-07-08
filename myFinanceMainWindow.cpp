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
    exchangeModel = new myExchangeListModel();

    ui->setupUi(this);

    ui->treeView->setModel(assetModel);
    //ui->treeView->setItemDelegate(delegate);
    //ui->treeView->header()->resizeSections(QHeaderView::ResizeToContents);
    ui->treeView->header()->resizeSections(QHeaderView::Fixed);
    ui->treeView->header()->resizeSection(0, 180);
    ui->treeView->header()->resizeSection(1, 110);
    ui->treeView->header()->resizeSection(2, 50);
    ui->treeView->expandAll();

    ui->listView->setModel(exchangeModel);

    connect(assetModel, SIGNAL(priceDataReflashed()), this, SLOT(priceDataReflashed()));
    connect(stockCode ,SIGNAL(codeDataReady()), this, SLOT(codeDataReady()));

    ui->statusBar->addWidget(&statusLabel);
    if (stockCode->getIsDataReady()) {
        statueStr_1 = STR("股票代码获取成功");
    } else {
        statueStr_1 = STR("");
    }
    statusLabel.setText(statueStr_1+statueStr_2);

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

    // mainToolBar
    exchange = new QAction(QIcon(":/icon/toolBar/resource/icon/toolBar/myExchange.png"), STR("资产变化"), this);
    connect(exchange, SIGNAL(triggered()), this, SLOT(on_exchange_clicked()));
    ui->mainToolBar->addAction(exchange);
    exchangeStock = new QAction(QIcon(":/icon/exchangeTab/resource/icon/exchangeTab/mySecurity.png"), STR("证券交易"), this);
    connect(exchangeStock, SIGNAL(triggered()), this, SLOT(on_exchangeStock_clicked()));
    ui->mainToolBar->addAction(exchangeStock);
    exchangeFund = new QAction(QIcon(":/icon/exchangeTab/resource/icon/exchangeTab/myFund.png"), STR("基金"), this);
    connect(exchangeFund, SIGNAL(triggered()), this, SLOT(on_exchangeFund_clicked()));
    ui->mainToolBar->addAction(exchangeFund);
    exchangeMoneyUp = new QAction(QIcon(":/icon/exchangeTab/resource/icon/exchangeTab/myMoneyUp.png"), STR("理财"), this);
    connect(exchangeMoneyUp, SIGNAL(triggered()), this, SLOT(on_exchangeMoneyUp_clicked()));
    ui->mainToolBar->addAction(exchangeMoneyUp);
    exchangeTransfer = new QAction(QIcon(":/icon/exchangeTab/resource/icon/exchangeTab/myTransfer.png"), STR("转帐"), this);
    connect(exchangeTransfer, SIGNAL(triggered()), this, SLOT(on_exchangeTransfer_clicked()));
    ui->mainToolBar->addAction(exchangeTransfer);
    ui->mainToolBar->addSeparator();

    newAccount = new QAction(QIcon(":/icon/toolBar/resource/icon/toolBar/myNewAccount.png"), STR("新建帐户"), this);
    connect(newAccount, SIGNAL(triggered()), this, SLOT(on_new_account_clicked()));
    ui->mainToolBar->addAction(newAccount);
    ui->mainToolBar->addSeparator();

    reflashData = new QAction(QIcon(":/icon/toolBar/resource/icon/toolBar/myReflashData.png"), STR("刷新"), this);
    connect(reflashData, SIGNAL(triggered()), this, SLOT(on_reflash_clicked()));
    ui->mainToolBar->addAction(reflashData);
    syncDatabase = new QAction(QIcon(":/icon/toolBar/resource/icon/toolBar/mySyncDatabase.png"), STR("界面与数据库同步"), this);
    connect(syncDatabase, SIGNAL(triggered()), this, SLOT(on_syncDatabase_clicked()));
    ui->mainToolBar->addAction(syncDatabase);
    ui->mainToolBar->addSeparator();

    updatePrice = new QAction(QIcon(":/icon/toolBar/resource/icon/toolBar/myUpdatePrice.png"), STR("更新价格"), this);;
    connect(updatePrice, SIGNAL(triggered()), this, SLOT(on_updatePrice_clicked()));
    ui->mainToolBar->addAction(updatePrice);
    hidePrice = new QAction(QIcon(":/icon/toolBar/resource/icon/toolBar/myHidePrice.png"), STR("更新价格"), this);;
    connect(hidePrice, SIGNAL(triggered()), this, SLOT(on_hidePrice_clicked()));
    ui->mainToolBar->addAction(hidePrice);
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
}

void myFinanceMainWindow::codeDataReady() {
    if (stockCode->getIsDataReady()) {
        reflashData->setEnabled(true);
    }
    statueStr_1 = STR("股票代码获取成功");
    statusLabel.setText(statueStr_1+statueStr_2);
    qDebug() << statueStr_1;
}

void myFinanceMainWindow::on_exchange_clicked()
{
    qDebug() << STR("资产变化 clicked");
    myExchangeUI exchangeUI = myExchangeUI();
    doExchange(exchangeUI);
}
void myFinanceMainWindow::on_exchangeStock_clicked()
{
    qDebug() << STR("证券交易 clicked");
    myExchangeUI exchangeUI(myExchangeUI::TAB_STOCK);
    doExchange(exchangeUI);
}
void myFinanceMainWindow::on_exchangeFund_clicked()
{
    qDebug() << STR("基金 clicked");
    myExchangeUI exchangeUI(myExchangeUI::TAB_FUNDS);
    doExchange(exchangeUI);
}
void myFinanceMainWindow::on_exchangeMoneyUp_clicked()
{
    qDebug() << STR("理财 clicked");
    myExchangeUI exchangeUI(myExchangeUI::TAB_MONUP);
    doExchange(exchangeUI);
}
void myFinanceMainWindow::on_exchangeTransfer_clicked()
{
    qDebug() << STR("转帐 clicked");
    myExchangeUI exchangeUI(myExchangeUI::TAB_TRANS);
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
    qDebug() << STR("刷新 clicked finished 正在读取股票代码");
    statueStr_1 = STR("正在读取股票代码");
    statusLabel.setText(statueStr_1+statueStr_2);
    reflashData->setEnabled(false);

    assetModel->doReflashData();
    stockCode->getStockCode();
    ui->treeView->expandAll();
}
void myFinanceMainWindow::on_syncDatabase_clicked() {
    assetModel->doReflashData();
    ui->treeView->expandAll();
}

void myFinanceMainWindow::on_updatePrice_clicked()
{
    qDebug() << STR("更新价格 clicked");
    assetModel->doUpdatePrice();
    ui->treeView->expandAll();
    qDebug() << STR("更新价格 clicked assetModel->doUpdatePrice requested");
}
void myFinanceMainWindow::priceDataReflashed() {
    ui->treeView->header()->resizeSection(0, 180);
    ui->treeView->header()->resizeSection(1, 110);
    ui->treeView->header()->resizeSection(2, 50);

    ui->treeView->expandAll();
    QDateTime dateTime = QDateTime::currentDateTime();
    float totalAsset = assetModel->doGetTotalAsset();
    QString strTotalAsset = QString::number(totalAsset, 'f', 2);
    float securityAsset = assetModel->doGetSecurityAsset();
    QString strSecurityAsset = QString::number(securityAsset, 'f', 2);
    statueStr_2 = STR("<b>【更新时间：%1●总资产：%2●证券账户：%3】</b>")
            .arg(dateTime.toString(STR("yyyy-MM-dd hh:mm:ss")))
            .arg(strTotalAsset).arg(strSecurityAsset);
    statusLabel.setText(statueStr_1+statueStr_2);
    qDebug() << STR("价格更新 finished") << statueStr_2;
}
void myFinanceMainWindow::on_hidePrice_clicked() {
    statueStr_2 = STR("");
    statusLabel.setText(statueStr_1+statueStr_2);
}

///////////////////////////////////////////////////////////////////////////////////////////
/// treeView的右键菜单
void myFinanceMainWindow::treeViewContextMenuSlot(const QPoint& pt) {
    QModelIndex index = ui->treeView->indexAt(pt);
    myIndexShell *node = assetModel->nodeFromIndex(index);
    treeViewContextMenu.treeViewContextMenu(node);
}
void myFinanceMainWindow::doChangeAssetDirectly(const myIndexShell *node, changeType type, const void *data, const QString &info, bool isFlash) {
    if (!assetModel->doChangeAssetDirectly(node, type, data, isFlash)) {
        ui->treeView->expandAll();
        QMessageBox::warning(this, info+"failed", info+"failed", QMessageBox::Ok, QMessageBox::Ok);
    }
    ui->treeView->expandAll();
}

void myFinanceMainWindow::doUpDown(const myIndexShell *node, bool isUp) {
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
    myExchangeUI tmpUI(originExchangeData, true);
    myFinanceExchangeWindow exWin(this, tmpUI, this, true);
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
    myExchangeUI tmpUI(originExchangeData, true);
    myFinanceExchangeWindow dial(this, tmpUI);
    dial.setWindowTitle(info);
    dial.setUI(originExchangeData, true);
    if(dial.exec() == QDialog::Accepted) {
        qDebug() << info + "Accepted";
        // 1. DO EXCHANGE ASSET_DATA
        if (dial.getIsRollback()) {
            ans = assetModel->doExchange(-originExchangeData) && ans;
        }
        // 2. DO EXCHANGE EXCHANGE_DATA
        if (false == ans) {
            QMessageBox::warning(this, info, info + " ERROR", QMessageBox::Ok, QMessageBox::Cancel);
        } else {
            exchangeModel->doExchange(originExchangeData, true);
        }
    }
    ui->treeView->expandAll();
}

void myFinanceMainWindow::doDividend(const myDividends &divident, const myAssetData &assetHold, bool isInterest, bool isRecordExchange) {
    myExchangeData exchangeData;
    if (isInterest) {
        exchangeData.exchangeType = STR("利息");
    } else {
        exchangeData.exchangeType = STR("分红");
    }
    assetModel->doDividend(divident, assetHold, exchangeData);
    if (isRecordExchange)
        exchangeModel->doExchange(exchangeData);

    ui->treeView->expandAll();
}
