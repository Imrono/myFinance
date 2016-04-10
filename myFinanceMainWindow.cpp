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
    ui(new Ui::myFinanceMainWindow), statusLabel(this),
    stockCode(myStockCodeName::getInstance()),
    assetModel(nullptr), exchangeModel(nullptr), editAsset(nullptr),
    deleteAsset(nullptr), insertAsset(nullptr), modifyAsset(nullptr)
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
        statusLabel.setText(QString::fromLocal8Bit("正在读取股票代码"));
    } else {
        ui->reflash->setEnabled(true);
        statusLabel.setText("ready");
    }

    /// treeView
    editAsset = new QMenu(this);
    deleteAsset = new QAction(ui->treeView);
    insertAsset = new QAction(ui->treeView);
    modifyAsset = new QAction(ui->treeView);
    upAsset     = new QAction(ui->treeView);;
    downAsset   = new QAction(ui->treeView);;
    connect(deleteAsset, SIGNAL(triggered()), this, SLOT(deleteAsset_clicked()));
    connect(insertAsset, SIGNAL(triggered()), this, SLOT(insertAsset_clicked()));
    connect(modifyAsset, SIGNAL(triggered()), this, SLOT(modifyAsset_clicked()));
    connect(upAsset    , SIGNAL(triggered()), this, SLOT(upAsset_clicked()));
    connect(downAsset  , SIGNAL(triggered()), this, SLOT(downAsset_clicked()));
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeView, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(treeViewContextMenu(const QPoint&)));
    /// listView
    editExchange = new QMenu(this);
    modifyExchange = new QAction(ui->listView);
    modifyExchange->setText(QString::fromLocal8Bit("更改交易"));
    deleteExchange = new QAction(ui->listView);
    deleteExchange->setText(QString::fromLocal8Bit("撤消交易"));
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
    if (nullptr != deleteExchange) {
        delete deleteExchange;
        deleteExchange = nullptr;
    }
    if (nullptr != modifyExchange) {
        delete modifyExchange;
        modifyExchange = nullptr;
    }
    if (nullptr != editExchange) {
        delete editExchange;
        editExchange = nullptr;
    }
    if (nullptr != upAsset) {
        delete upAsset;
        upAsset = nullptr;
    }
    if (nullptr != downAsset) {
        delete downAsset;
        downAsset = nullptr;
    }
    if (nullptr != modifyAsset) {
        delete modifyAsset;
        modifyAsset = nullptr;
    }
    if (nullptr != insertAsset) {
        delete insertAsset;
        insertAsset = nullptr;
    }
    if (nullptr != deleteAsset) {
        delete deleteAsset;
        deleteAsset = nullptr;
    }
    if (nullptr != editAsset) {
        delete editAsset;
        editAsset = nullptr;
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
    myFinanceExchangeWindow exWin(this);
    if(exWin.exec() == QDialog::Accepted) {
        myExchangeData data = exWin.getExchangeData();
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
    myInsertModifyAccount dial(this);
    if(dial.exec() == QDialog::Accepted) {
        if(!assetModel->doInsertAccount(dial.getData())) {
            ui->treeView->expandAll();
            QMessageBox::warning(this, QString::fromLocal8Bit("新建帐户错误"),
                                 QString::fromLocal8Bit("新建帐户错误"),
                                 QMessageBox::Ok, QMessageBox::Ok);
        }
    } else {
        qDebug() << QString::fromLocal8Bit("添加帐户 Canceled");
    }
    ui->treeView->expandAll();
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

void myFinanceMainWindow::treeViewContextMenu(const QPoint& pt) {
    Q_UNUSED(pt);
    myAssetNode *node = assetModel->nodeFromIndex(ui->treeView->currentIndex());
    editAsset->clear();
    if (myAssetNode::nodeAccount == node->type) {
        insertAsset->setText(QString::fromLocal8Bit("添加资产"));
        editAsset->addAction(insertAsset);
        modifyAsset->setText(QString::fromLocal8Bit("更新帐户"));
        editAsset->addAction(modifyAsset);
        deleteAsset->setText(QString::fromLocal8Bit("删除帐户"));
        editAsset->addAction(deleteAsset);
    } else if (myAssetNode::nodeHolds == node->type) {
        modifyAsset->setText(QString::fromLocal8Bit("更新资产"));
        editAsset->addAction(modifyAsset);
        deleteAsset->setText(QString::fromLocal8Bit("删除资产"));
        editAsset->addAction(deleteAsset);
    } else if (myAssetNode::nodeRoot == node->type) {
    } else {}

    upAsset->setEnabled(true);
    downAsset->setEnabled(true);
    unsigned int upDownType = myFinanceMainWindow::HAS_NONE;
    if (myAssetNode::nodeAccount == node->type) {
        int pos = node->nodeData.value<myAssetAccount>().pos;
        if (0 != pos) {
            upDownType |= myFinanceMainWindow::HAS_UP;
        }
        if (node->parent->children.count()-1 != pos){
            upDownType |= myFinanceMainWindow::HAS_DOWN;
        }
    } else if (myAssetNode::nodeHolds == node->type) {
        int pos = node->nodeData.value<myAssetHold>().pos;
        if (0 != pos) {
            upDownType |= myFinanceMainWindow::HAS_UP;
        }
        if (node->parent->children.count()-1 != pos){
            upDownType |= myFinanceMainWindow::HAS_DOWN;
        }
    } else {}

    editAsset->addSeparator();
    upAsset->setText(QString::fromLocal8Bit("上移"));
    editAsset->addAction(upAsset);
    downAsset->setText(QString::fromLocal8Bit("下移"));
    editAsset->addAction(downAsset);
    if (!(upDownType & myFinanceMainWindow::HAS_UP)) {
        upAsset->setDisabled(true);
    }
    if (!(upDownType & myFinanceMainWindow::HAS_DOWN)) {
        downAsset->setDisabled(true);
    }
    editAsset->exec(QCursor::pos());
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
void myFinanceMainWindow::upAsset_clicked() {
    doUpDown(true);
}
void myFinanceMainWindow::downAsset_clicked() {
    doUpDown(false);
}

void myFinanceMainWindow::doChangeAssetDirectly(changeType type) {
    myAssetNode *node = assetModel->nodeFromIndex(ui->treeView->currentIndex());
    QVariant data;
     QString info;

    if (myAssetNode::nodeAccount == node->type) {
        /// INSERT ASSET
        if (POP_INSERT == type) {
            info = QString::fromLocal8Bit("添加资产");
            myAssetAccount nodeData = node->nodeData.value<myAssetAccount>();
            myInsertModifyAsset dial(nodeData.code, nodeData.name, this);
            dial.setWindowTitle(info);
            if(dial.exec() == QDialog::Accepted) {
                data.setValue(dial.getData());
                qDebug() << info + "Accepted";
            } else {
                return;
            }
        /// MODIFY ACCOUNT
        } else if (POP_MODIFY == type) {
            info = QString::fromLocal8Bit("更新帐户");
            myAssetAccount nodeData = node->nodeData.value<myAssetAccount>();
            myAccountData originAccountData(nodeData);
            myInsertModifyAccount dial(this);
            dial.setWindowTitle(info);
            dial.setUI(myAccountData(nodeData));
            if(dial.exec() == QDialog::Accepted) {
                qDebug() << info + "Accepted";
                myAccountData targetAccountData = dial.getData();
                if (myAccountData::isSameAccountData(targetAccountData, originAccountData)) {
                    qDebug() << info + "Nothing Changed";
                    return;
                } else {
                    data.setValue(targetAccountData);
                }
            } else {
                return;
            }
        /// DELETE ACCOUNT
        } else if (POP_DELETE == type) {
            info = QString::fromLocal8Bit("删除帐户");
            if(QMessageBox::Ok == QMessageBox::warning(this, info, info + "->\n" +
                                  node->nodeData.value<myAssetAccount>().code + "\n" + node->nodeData.value<myAssetAccount>().name,
                                  QMessageBox::Ok|QMessageBox::Cancel, QMessageBox::Ok)) {
                qDebug() << info + "Accepted";
            } else {
                return;
            }
        } else {}
    } else if (myAssetNode::nodeHolds == node->type) {
        /// MODIFY ASSET
        if (POP_MODIFY == type) {
            info = QString::fromLocal8Bit("更新资产");
            myAssetHold nodeData = node->nodeData.value<myAssetHold>();
            myAssetData originAssetData(nodeData);
            myAssetNode *accountNode = assetModel->getRootNode().getAccountNode(nodeData.accountCode);
            myAssetAccount accountNodeData = accountNode->nodeData.value<myAssetAccount>();
            myInsertModifyAsset dial(accountNodeData.code, accountNodeData.name, this);
            dial.setUI(myAssetData(nodeData));
            dial.setWindowTitle(info);
            if(dial.exec() == QDialog::Accepted) {
                qDebug() << info + "Accepted";
                myAssetData targetAssetData = dial.getData();
                if (myAssetData::isSameAssetData(targetAssetData, originAssetData)) {
                    qDebug() << info + "Nothing Changed";
                    return;
                } else {
                    data.setValue(targetAssetData);
                }
            } else {
                return;
            }
        /// DELETE ASSET
        } else if (POP_DELETE == type) {
            info = QString::fromLocal8Bit("删除资产");
            if(QMessageBox::Ok == QMessageBox::warning(this, info, info + "->\n" +
                                  node->nodeData.value<myAssetHold>().assetCode + "\n" + node->nodeData.value<myAssetHold>().name,
                                  QMessageBox::Ok|QMessageBox::Cancel, QMessageBox::Ok)) {
                qDebug() << info + "Accepted";
            } else {
                return;
            }
        } else {}
    } else if (myAssetNode::nodeRoot == node->type) {
    } else {}

    qDebug() << "view" << (int)type;
    if (!assetModel->doChangeAssetDirectly(node, type, data)) {
        ui->treeView->expandAll();
        QMessageBox::warning(this, info+"failed", info+"failed", QMessageBox::Ok, QMessageBox::Ok);
    }
    ui->treeView->expandAll();
}

void myFinanceMainWindow::doUpDown(bool isUp) {
    myAssetNode *node = assetModel->nodeFromIndex(ui->treeView->currentIndex());
    if (!assetModel->doUpDown(isUp, node)) {
        ui->treeView->expandAll();
        QMessageBox::warning(this, "doUpDown failed", "doUpDown failed", QMessageBox::Ok, QMessageBox::Ok);
    }
    ui->treeView->expandAll();
}

void myFinanceMainWindow::listViewContextMenu(const QPoint& pt) {
    Q_UNUSED(pt);

    editExchange->exec(QCursor::pos());
}
void myFinanceMainWindow::modifyExchange_clicked() {
    bool ans = true;
    QString info = QString::fromLocal8Bit("更改资产变化");
    int line = ui->listView->currentIndex().row();
    myExchangeData originExchangeData = exchangeModel->getDataFromRow(line);
    myFinanceExchangeWindow exWin(this);
    exWin.setWindowTitle(info);
    exWin.setUI(originExchangeData, true);
    if(exWin.exec() == QDialog::Accepted) {
        qDebug() << info + "Accepted";
        // 1. DO EXCHANGE ASSET_DATA
        myExchangeData targetExchangeData = exWin.getExchangeData();
        myExchangeData doExchangeData = targetExchangeData;
        int type = myExchangeListModel::NO_DO_EXCHANGE;
        exchangeModel->coordinatorModifyExchange(originExchangeData, targetExchangeData, type);
        if (exWin.getIsRollback()) {
            bool isMoneyChange = false, isAssetChange = false;
            isMoneyChange = type&myExchangeListModel::ORIG_ACCOUNT_1;
            isAssetChange = type&myExchangeListModel::ORIG_ACCOUNT_2;
            ans = assetModel->doExchange(-originExchangeData, isMoneyChange, isAssetChange, false) && ans;
            isMoneyChange = type&myExchangeListModel::TARG_ACCOUNT_1;
            isAssetChange = type&myExchangeListModel::TARG_ACCOUNT_2;
            ans = assetModel->doExchange(targetExchangeData, isMoneyChange, isAssetChange, true) && ans;
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
    QString info = QString::fromLocal8Bit("撤消资产变化");
    int line = ui->listView->currentIndex().row();
    myExchangeData originExchangeData = exchangeModel->getDataFromRow(line);
    myModifyExchange dial(this);
    dial.setWindowTitle(info);
    dial.setUI(originExchangeData);
    dial.setUI4Delete();
    if(dial.exec() == QDialog::Accepted) {
        qDebug() << info + "Accepted";
        // 1. DO EXCHANGE ASSET_DATA
        if (dial.isRollback()) {
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
