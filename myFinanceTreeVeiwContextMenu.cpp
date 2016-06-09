#include "myFinanceTreeVeiwContextMenu.h"

#include <QModelIndex>
#include <QMessageBox>

#include "myInsertModifyAccount.h"
#include "myInsertModifyAsset.h"
#include "myModifyExchange.h"
#include "myFinanceMainWindow.h"

myFinanceTreeVeiwContextMenu::myFinanceTreeVeiwContextMenu(QWidget *parent) : QMenu(parent), parent(static_cast<myFinanceMainWindow *>(parent)) {
    editAsset = new QMenu(this);

    deleteAccount = new QAction(this);
    modifyAccount = new QAction(this);
    insertAsset   = new QAction(this);
    deleteAsset   = new QAction(this);
    modifyAsset   = new QAction(this);
    buyAsset      = new QAction(this);
    sellAsset     = new QAction(this);
    transferIn    = new QAction(this);
    transferOut   = new QAction(this);
    upAsset       = new QAction(this);
    downAsset     = new QAction(this);

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
}

void myFinanceTreeVeiwContextMenu::treeViewContextMenu(const myAssetNode *node) {
    currentNode = node;

    editAsset->clear();
    if (myAssetNode::nodeAccount == node->type) {
        insertAsset->setText(STR("添加资产"));
        editAsset->addAction(insertAsset);
        modifyAsset->setText(STR("更新帐户"));
        editAsset->addAction(modifyAsset);
        deleteAsset->setText(STR("删除帐户"));
        editAsset->addAction(deleteAsset);
    } else if (myAssetNode::nodeHolds == node->type) {
        modifyAsset->setText(STR("更新资产"));
        editAsset->addAction(modifyAsset);
        deleteAsset->setText(STR("删除资产"));
        editAsset->addAction(deleteAsset);
        editAsset->addSeparator();
        myAssetHold assetHolds = node->nodeData.value<myAssetHold>();
        if (assetHolds.assetCode == STR("cash")) {
            editAsset->addAction(transferIn);
            editAsset->addAction(transferOut);
        } else {
            editAsset->addAction(buyAsset);
            editAsset->addAction(sellAsset);
        }
    } else if (myAssetNode::nodeRoot == node->type) {
    } else {}

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

    editAsset->addSeparator();
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

void myFinanceTreeVeiwContextMenu::deleteAsset_clicked() {
    doChangeAssetDirectly(POP_DELETE);
}

void myFinanceTreeVeiwContextMenu::insertAsset_clicked() {
    doChangeAssetDirectly(POP_INSERT);
}

void myFinanceTreeVeiwContextMenu::modifyAsset_clicked() {
    doChangeAssetDirectly(POP_MODIFY);
}

void myFinanceTreeVeiwContextMenu::buyAsset_clicked() {

}

void myFinanceTreeVeiwContextMenu::sellAsset_clicked() {

}

void myFinanceTreeVeiwContextMenu::transferIn_clicked() {

}

void myFinanceTreeVeiwContextMenu::transferOut_clicked() {

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
            info = STR("更新帐户");
            myAssetAccount nodeData = currentNode->nodeData.value<myAssetAccount>();
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
            info = STR("删除帐户");
            if(QMessageBox::Ok == QMessageBox::warning(this, info, info + "->\n" +
                                  currentNode->nodeData.value<myAssetAccount>().code + "\n" + currentNode->nodeData.value<myAssetAccount>().name,
                                  QMessageBox::Ok|QMessageBox::Cancel, QMessageBox::Ok)) {
                qDebug() << info + "Accepted";
            } else {
                return;
            }
        } else {}
    } else if (myAssetNode::nodeHolds == currentNode->type) {
        myAssetHold nodeData = currentNode->nodeData.value<myAssetHold>();
        myAssetData originAssetData(nodeData);
        myAssetAccount accountNodeData = currentNode->parent->nodeData.value<myAssetAccount>();
        myInsertModifyAsset dial(accountNodeData.code, accountNodeData.name, this);
        dial.setUI(myAssetData(nodeData));
        /// MODIFY ASSET
        if (POP_MODIFY == type) {
            info = STR("更新资产");
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
