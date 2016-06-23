#include "myFinanceTreeVeiwContextMenu.h"

#include <QModelIndex>
#include <QMessageBox>

#include "myInsertModifyAccount.h"
#include "myInsertModifyAsset.h"
#include "myModifyExchange.h"
#include "myFinanceMainWindow.h"
#include "myFinanceExchangeWindow.h"
#include "myDividendsDialog.h"

myFinanceTreeVeiwContextMenu::myFinanceTreeVeiwContextMenu(QWidget *parent) : parent(static_cast<myFinanceMainWindow *>(parent)) {
    editAsset = new QMenu(parent);

    deleteAccount = new QAction(parent);
    modifyAccount = new QAction(parent);
    insertAsset   = new QAction(parent);
    deleteAsset   = new QAction(parent);
    modifyAsset   = new QAction(parent);
    buyAsset      = new QAction(parent);
    sellAsset     = new QAction(parent);
    transferIn    = new QAction(parent);
    transferOut   = new QAction(parent);
    upAsset       = new QAction(parent);
    downAsset     = new QAction(parent);
    stockBonus    = new QAction(parent);
    intrests      = new QAction(parent);

    deleteAccount->setText(STR("删除帐户"));
    modifyAccount->setText(STR("更新帐户"));
    insertAsset->setText(STR("添加资产"));
    deleteAsset->setText(STR("删除资产"));
    modifyAsset->setText(STR("更新资产"));
    buyAsset->setText(STR("买入"));
    sellAsset->setText(STR("卖出"));
    transferIn->setText(STR("转入"));
    transferOut->setText(STR("转出"));
    upAsset->setText(STR("上移"));
    downAsset->setText(STR("下移"));
    stockBonus->setText(STR("分红"));
    intrests->setText(STR("利息"));

    connect(deleteAccount, SIGNAL(triggered()), this, SLOT(deleteAccount_clicked()));
    connect(modifyAccount, SIGNAL(triggered()), this, SLOT(modifyAccount_clicked()));
    connect(insertAsset,   SIGNAL(triggered()), this, SLOT(insertAsset_clicked()));
    connect(deleteAsset,   SIGNAL(triggered()), this, SLOT(deleteAsset_clicked()));
    connect(modifyAsset,   SIGNAL(triggered()), this, SLOT(modifyAsset_clicked()));
    connect(buyAsset,      SIGNAL(triggered()), this, SLOT(buyAsset_clicked()));
    connect(sellAsset,     SIGNAL(triggered()), this, SLOT(sellAsset_clicked()));
    connect(transferIn,    SIGNAL(triggered()), this, SLOT(transferIn_clicked()));
    connect(transferOut,   SIGNAL(triggered()), this, SLOT(transferOut_clicked()));
    connect(upAsset,       SIGNAL(triggered()), this, SLOT(upAsset_clicked()));
    connect(downAsset,     SIGNAL(triggered()), this, SLOT(downAsset_clicked()));
    connect(stockBonus,    SIGNAL(triggered()), this, SLOT(stockBonus_clicked()));
    connect(intrests,      SIGNAL(triggered()), this, SLOT(intrests_clicked()));
}
myFinanceTreeVeiwContextMenu::~myFinanceTreeVeiwContextMenu() {

}

void myFinanceTreeVeiwContextMenu::treeViewContextMenu(const myAssetNode *node) {
    currentNode = node;

    editAsset->clear();
    if (myAssetNode::nodeAccount == node->type) {
        editAsset->addAction(insertAsset);
        editAsset->addAction(modifyAccount);
        editAsset->addAction(deleteAccount);
    } else if (myAssetNode::nodeHolds == node->type) {
        editAsset->addAction(modifyAsset);
        editAsset->addAction(deleteAsset);
        editAsset->addSeparator();
        myAssetHold assetHolds = node->nodeData.value<myAssetHold>();
        if (assetHolds.assetData.assetCode == MY_CASH) {
            editAsset->addAction(transferIn);
            editAsset->addAction(transferOut);
        } else {
            editAsset->addAction(buyAsset);
            editAsset->addAction(sellAsset);
        }
        editAsset->addSeparator();
        if (assetHolds.assetData.assetCode == MY_CASH
         || assetHolds.assetData.assetCode == STR("货币基金")) {
            editAsset->addAction(intrests);
        } else {
            editAsset->addAction(stockBonus);
        }
    } else if (myAssetNode::nodeRoot == node->type) {
    } else {}

    editAsset->addSeparator();
    upAsset->setEnabled(true);
    downAsset->setEnabled(true);
    unsigned int upDownType = HAS_NONE;
    if (myAssetNode::nodeAccount == node->type) {
        int pos = node->nodeData.value<myAssetAccount>().pos;
        if (0 != pos) {
            upDownType |= HAS_UP;
        }
        if (node->parent->children.count()-1 != pos){
            upDownType |= HAS_DOWN;
        }
    } else if (myAssetNode::nodeHolds == node->type) {
        int pos = node->nodeData.value<myAssetHold>().pos;
        if (0 != pos) {
            upDownType |= HAS_UP;
        }
        if (node->parent->children.count()-1 != pos){
            upDownType |= HAS_DOWN;
        }
    } else {}

    upAsset->setText(STR("上移"));
    editAsset->addAction(upAsset);
    downAsset->setText(STR("下移"));
    editAsset->addAction(downAsset);
    if (!(upDownType & HAS_UP)) {
        upAsset->setDisabled(true);
    }
    if (!(upDownType & HAS_DOWN)) {
        downAsset->setDisabled(true);
    }
    editAsset->exec(QCursor::pos());
}

void myFinanceTreeVeiwContextMenu::deleteAccount_clicked() {
    doChangeAssetDirectly(POP_DELETE);
}
void myFinanceTreeVeiwContextMenu::modifyAccount_clicked() {
    doChangeAssetDirectly(POP_MODIFY);
}
void myFinanceTreeVeiwContextMenu::insertAsset_clicked() {
    doChangeAssetDirectly(POP_INSERT);
}
void myFinanceTreeVeiwContextMenu::deleteAsset_clicked() {
    doChangeAssetDirectly(POP_DELETE);
}
void myFinanceTreeVeiwContextMenu::modifyAsset_clicked() {
    doChangeAssetDirectly(POP_MODIFY);
}

void myFinanceTreeVeiwContextMenu::buyAsset_clicked() {
    qDebug() << STR("右键买入 clicked");
    doExchangeStock(STR("证券买入"));
}
void myFinanceTreeVeiwContextMenu::sellAsset_clicked() {
    qDebug() << STR("右键卖出 clicked");
    doExchangeStock(STR("证券卖出"));
}
void myFinanceTreeVeiwContextMenu::doExchangeStock(const QString &type) {
    myExchangeData exchangeData;
    myAssetHold holds = currentNode->nodeData.value<myAssetHold>();
    exchangeData.accountMoney          = holds.assetData.accountCode;
    exchangeData.assetData.accountCode = holds.assetData.accountCode;
    exchangeData.assetData.assetCode   = holds.assetData.assetCode;
    exchangeData.assetData.assetName   = holds.assetData.assetName;
    exchangeData.exchangeType          = type;
    parent->doExchange(myExchangeUI(exchangeData, false), true);
}

void myFinanceTreeVeiwContextMenu::transferIn_clicked() {
    qDebug() << STR("右键转入 clicked");
    myExchangeData exchangeData;
    myAssetHold holds = currentNode->nodeData.value<myAssetHold>();
    exchangeData.assetData.accountCode = holds.assetData.accountCode;
    exchangeData.exchangeType = STR("转帐");
    parent->doExchange(myExchangeUI(exchangeData, false), true);
}

void myFinanceTreeVeiwContextMenu::transferOut_clicked() {
    qDebug() << STR("右键转出 clicked");
    myExchangeData exchangeData;
    myAssetHold holds = currentNode->nodeData.value<myAssetHold>();
    exchangeData.accountMoney = holds.assetData.accountCode;
    exchangeData.exchangeType = STR("转帐");
    parent->doExchange(myExchangeUI(exchangeData, false), true);
}

void myFinanceTreeVeiwContextMenu::upAsset_clicked() {
    doUpDown(true);
}
void myFinanceTreeVeiwContextMenu::downAsset_clicked() {
    doUpDown(false);
}


void myFinanceTreeVeiwContextMenu::doChangeAssetDirectly(changeType type) {
    QVariant data;
    QString info;

    if (myAssetNode::nodeAccount == currentNode->type) {
        /// INSERT ASSET
        if (POP_INSERT == type) {
            info = STR("添加资产");
            myAssetAccount nodeData = currentNode->nodeData.value<myAssetAccount>();
            myInsertModifyAsset dial(nodeData.accountData.code, nodeData.accountData.name, parent);
            dial.setWindowTitle(info);
            if(dial.exec() == QDialog::Accepted) {
                data.setValue(dial.getData());
                qDebug() << info + "Accepted";
            } else {
                return;
            }
        /// MODIFY ACCOUNT
        } else if (POP_MODIFY == type) {
            info = STR("更新帐户");
            myAssetAccount nodeData = currentNode->nodeData.value<myAssetAccount>();
            myAccountData originAccountData(nodeData);
            myInsertModifyAccount dial(parent);
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
            info = STR("删除帐户");
            if(QMessageBox::Ok == QMessageBox::warning(parent, info, info + "->\n" +
                                  currentNode->nodeData.value<myAssetAccount>().accountData.code + "\n" + currentNode->nodeData.value<myAssetAccount>().accountData.name,
                                  QMessageBox::Ok|QMessageBox::Cancel, QMessageBox::Ok)) {
                qDebug() << info + "Accepted";
            } else {
                return;
            }
        } else {}
    } else if (myAssetNode::nodeHolds == currentNode->type) {
        myAssetHold nodeData = currentNode->nodeData.value<myAssetHold>();
        myAssetAccount accountNodeData = currentNode->parent->nodeData.value<myAssetAccount>();
        myInsertModifyAsset dial(accountNodeData.accountData.code, accountNodeData.accountData.name, parent);
        dial.setUI(myAssetData(nodeData));
        /// MODIFY ASSET
        if (POP_MODIFY == type) {
            info = STR("更新资产");
            dial.setWindowTitle(info);
            if(dial.exec() == QDialog::Accepted) {
                qDebug() << info + "Accepted";
                myAssetData originAssetData(nodeData);
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
            info = STR("删除资产");
            dial.setWindowTitle(info);
            dial.setAllDisabled();
            if(dial.exec() == QDialog::Accepted) {
                qDebug() << info + "Accepted";
            } else {
                return;
            }
        } else {}
    } else if (myAssetNode::nodeRoot == currentNode->type) {
    } else {}

    qDebug() << "view" << (int)type;
    parent->doChangeAssetDirectly(currentNode, type, data, info);

}
void myFinanceTreeVeiwContextMenu::doUpDown(bool isUp) {
    parent->doUpDown(currentNode, isUp);
}

void myFinanceTreeVeiwContextMenu::stockBonus_clicked() {
    qDebug() << STR("右键分红 clicked");
    stockBonus_intrests(false);
}
void myFinanceTreeVeiwContextMenu::intrests_clicked() {
    qDebug() << STR("右键利息 clicked");
    stockBonus_intrests(true);
}
void myFinanceTreeVeiwContextMenu::stockBonus_intrests(bool isIntrest) {
    myAssetHold holds = currentNode->nodeData.value<myAssetHold>();
    myDividendsDialog dial(holds.assetData, isIntrest, parent);
    if(dial.exec() == QDialog::Accepted) {
        myDividends dividendsData = dial.getDividendsData();
        qDebug() << "base" << dividendsData.base
                 << "shareSplit" << dividendsData.shareSplit
                 << "shareBonus" << dividendsData.shareBonus
                 << "capitalBonus" << dividendsData.capitalBonus
                 << "time" << dividendsData.time.toString()
                 << "type" << dividendsData.type;
        parent->doDividend(dividendsData, holds.assetData, isIntrest);
    }
}
