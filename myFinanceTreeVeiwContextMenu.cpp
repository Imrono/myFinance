#include "myFinanceTreeVeiwContextMenu.h"

#include <QModelIndex>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>

#include "myInsertModifyAccount.h"
#include "myInsertModifyAsset.h"
#include "myModifyExchange.h"
#include "myFinanceMainWindow.h"
#include "myFinanceExchangeWindow.h"
#include "myDividendsDialog.h"

myFinanceTreeVeiwContextMenu::myFinanceTreeVeiwContextMenu(QWidget *parent) : parent(static_cast<myFinanceMainWindow *>(parent)) {
    editAsset = new QMenu(parent);

    deleteAccount   = new QAction(parent);
    modifyAccount   = new QAction(parent);
    insertAsset     = new QAction(parent);
    addAssetList    = new QAction(parent);
    addExchangeList = new QAction(parent);
    undoExchangeList= new QAction(parent);

    deleteAsset     = new QAction(parent);
    modifyAsset     = new QAction(parent);
    buyAsset        = new QAction(parent);
    sellAsset       = new QAction(parent);
    transferIn      = new QAction(parent);
    transferOut     = new QAction(parent);
    upAsset         = new QAction(parent);
    downAsset       = new QAction(parent);
    stockBonus      = new QAction(parent);
    intrests        = new QAction(parent);

    deleteAccount->setText(STR("删除帐户"));
    modifyAccount->setText(STR("更新帐户"));
    insertAsset->setText(STR("添加资产"));
    addAssetList->setText(STR("从文件添加资产"));
    addExchangeList->setText(STR("从文件读入交易数据"));
    undoExchangeList->setText(STR("undo last读入交易数据"));

    upAsset->setText(STR("上移"));
    downAsset->setText(STR("下移"));

    deleteAsset->setText(STR("删除资产"));
    modifyAsset->setText(STR("更新资产"));
    buyAsset->setText(STR("买入"));
    sellAsset->setText(STR("卖出"));
    transferIn->setText(STR("转入"));
    transferOut->setText(STR("转出"));
    stockBonus->setText(STR("分红"));
    intrests->setText(STR("利息"));

    connect(deleteAccount,   SIGNAL(triggered()), this, SLOT(deleteAccount_clicked()));
    connect(modifyAccount,   SIGNAL(triggered()), this, SLOT(modifyAccount_clicked()));
    connect(insertAsset,     SIGNAL(triggered()), this, SLOT(insertAsset_clicked()));
    connect(addAssetList,    SIGNAL(triggered()), this, SLOT(addAssetList_clicked()));
    connect(addExchangeList, SIGNAL(triggered()), this, SLOT(addExchangeList_clicked()));
    connect(undoExchangeList,SIGNAL(triggered()), this, SLOT(undoExchangeList_clicked()));

    connect(upAsset,       SIGNAL(triggered()), this, SLOT(upAsset_clicked()));
    connect(downAsset,     SIGNAL(triggered()), this, SLOT(downAsset_clicked()));

    connect(deleteAsset,   SIGNAL(triggered()), this, SLOT(deleteAsset_clicked()));
    connect(modifyAsset,   SIGNAL(triggered()), this, SLOT(modifyAsset_clicked()));
    connect(buyAsset,      SIGNAL(triggered()), this, SLOT(buyAsset_clicked()));
    connect(sellAsset,     SIGNAL(triggered()), this, SLOT(sellAsset_clicked()));
    connect(transferIn,    SIGNAL(triggered()), this, SLOT(transferIn_clicked()));
    connect(transferOut,   SIGNAL(triggered()), this, SLOT(transferOut_clicked()));
    connect(stockBonus,    SIGNAL(triggered()), this, SLOT(stockBonus_clicked()));
    connect(intrests,      SIGNAL(triggered()), this, SLOT(intrests_clicked()));
}
myFinanceTreeVeiwContextMenu::~myFinanceTreeVeiwContextMenu() {

}

void myFinanceTreeVeiwContextMenu::treeViewContextMenu(const myIndexShell *node) {
    currentNode = node;

    editAsset->clear();
    if (myAssetNode::nodeAccount == node->type) {
        editAsset->addAction(insertAsset);
        editAsset->addAction(modifyAccount);
        editAsset->addAction(deleteAccount);
        editAsset->addSeparator();

        editAsset->addAction(addAssetList);
        editAsset->addAction(addExchangeList);
        editAsset->addAction(undoExchangeList);
        qDebug() << "currentNode->children.count():" << currentNode->children.count() << "type:" << currentNode->type;
        if (0 != currentNode->children.count()) {
            addAssetList->setDisabled(true);
        } else {
            addAssetList->setEnabled(true);
        }
        if (0 == lastExchangeList.count()) {
            undoExchangeList->setDisabled(true);
        } else {
            undoExchangeList->setEnabled(true);
        }
    } else if (myAssetNode::nodeHolds == node->type) {
        const myAssetNodeData &assetHolds = GET_CONST_ASSET_NODE_DATA(node);
        editAsset->addAction(modifyAsset);
        editAsset->addAction(deleteAsset);
        editAsset->addSeparator();
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
        int pos = GET_CONST_ACCOUNT_NODE_DATA(node).pos;
        if (0 != pos) {
            upDownType |= HAS_UP;
        }
        if (node->parent->children.count()-1 != pos){
            upDownType |= HAS_DOWN;
        }
    } else if (myAssetNode::nodeHolds == node->type) {
        int pos = GET_CONST_ASSET_NODE_DATA(node).pos;
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
void myFinanceTreeVeiwContextMenu::addAssetList_clicked() {
    // 1. 读取并保存资产数据
    QList<myAssetData> assetDataList;
    QString fileName = QFileDialog::getOpenFileName(parent, tr("open file"), " ",  tr("Allfile(*.*)"));
    qDebug() << "fileName:" << fileName;
    if (!analyzeStockFromFile(fileName, assetDataList)) {
        qDebug() << "analyzeStockFromFile failed!!";
        return;
    }

    // 2. 批量添加资产
    int assetCount = assetDataList.count();
    QString info = STR("从文件批量添加资产%1项").arg(assetCount);
    qDebug() << info;
    for (int i = 0; i < assetCount; i++) {
        parent->doChangeAssetDirectly(currentNode, POP_INSERT, (const void *)&assetDataList[i], info, assetCount-1 == i);
    }
}
bool myFinanceTreeVeiwContextMenu::analyzeStockFromFile(const QString &fileName, QList<myAssetData> &assetDataList) {
    const myAccountNode *account = static_cast<const myAccountNode *>(currentNode);
    if (nullptr == account)
        return false;

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        qDebug() << "can not open file: " << fileName;
        return false;
    }

    QTextStream txtGet(&file);
    QString lineStr;
    bool flag = false;
    QMap<int, int> dataIdx2fileDataIdx;

    while(!txtGet.atEnd())
    {
        lineStr = txtGet.readLine();
        lineStr.remove(" ");
        qDebug() << lineStr;
        if (lineStr.contains(STR(",,,,,,,,合计(人民币)")))
            break;
        QStringList strList = lineStr.split(',');

        if (!flag) {
            if (lineStr.contains(STR("余额")) && lineStr.contains(STR("总资产"))) {
                lineStr = txtGet.readLine();
                lineStr.remove(" ");
                qDebug() << lineStr << endl;
                int left = lineStr.indexOf("\"");
                int rigth = lineStr.mid(left+1).indexOf("\"");
                QString money = lineStr.mid(left+1).left(rigth);
                money.remove(",");

                myAssetData assetData;
                assetData.accountCode = account->dbAccountData.accountData.code;
                assetData.assetCode   = MY_CASH;
                assetData.assetName   = STR("现金");
                assetData.amount      = 1;
                assetData.price       = money.toDouble();
                assetData.type        = STR("现金");

                assetDataList.append(assetData);
            }
            if (lineStr.contains(STR("证券代码"))) {
                flag = true;
                for (int i = 0; i < strList.count(); i++) {
                    qDebug() << strList.at(i);
                    if (strList.at(i) == STR("证券代码"))
                        dataIdx2fileDataIdx[0] = i;
                    if (strList.at(i) == STR("证券名称"))
                        dataIdx2fileDataIdx[1] = i;
                    if (strList.at(i) == STR("股票余额"))
                        dataIdx2fileDataIdx[2] = i;
                    if (strList.at(i) == STR("成本价") || strList.at(i) == STR("盈亏成本"))
                        dataIdx2fileDataIdx[3] = i;
                }
            }
        } else {
            myAssetData assetData;
            assetData.accountCode = account->dbAccountData.accountData.code;
            assetData.assetCode   = assetCodeWithMarket(strList.at(dataIdx2fileDataIdx[0]));
            assetData.assetName   = strList.at(dataIdx2fileDataIdx[1]);
            assetData.amount      = strList.at(dataIdx2fileDataIdx[2]).toDouble();
            assetData.price       = strList.at(dataIdx2fileDataIdx[3]).toDouble();
            assetData.type        = STR("股票");

            assetDataList.append(assetData);
        }
    }
    file.close();
    return true;
}

void myFinanceTreeVeiwContextMenu::addExchangeList_clicked() {
    // 1. 读取并保存资产数据
    QList<myExchangeData> exchangeDataList;
    QString fileName = QFileDialog::getOpenFileName(parent, tr("open file"), " ",  tr("Allfile(*.*)"));
    qDebug() << "fileName:" << fileName;
    if (!analyzeExchangeFromFile(fileName, exchangeDataList)) {
        qDebug() << "analyzeStockFromFile failed!!";
        return;
    }

    // 2. 批量添加资产变化
    lastExchangeList.clear();
    int insertCount = exchangeDataList.count();
    QString info = STR("从文件批量添加资产变化%1项").arg(insertCount);
    qDebug() << info;
    for (int i = 0; i < insertCount; i++) {
        myExchangeData exchangeData = exchangeDataList.at(i);
        parent->exchangeModel->doExchange(exchangeData, false, i == insertCount-1);
        qDebug() << "insert exchangeData.id" << exchangeData.id;
        lastExchangeList.append(exchangeData.id);
    }
}
bool myFinanceTreeVeiwContextMenu::analyzeExchangeFromFile(const QString &fileName, QList<myExchangeData> &exchangeDataList) {
    const myAccountNode *account = static_cast<const myAccountNode *>(currentNode);
    if (nullptr == account)
        return false;

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        qDebug() << "can not open file: " << fileName;
        return false;
    }

    QTextStream txtGet(&file);
    QString lineStr;
    bool flag = false;
    QMap<int, int> dataIdx2fileDataIdx;

    while(!txtGet.atEnd())
    {
        lineStr = txtGet.readLine();
        lineStr.remove(" ");
        if ("" == lineStr)
            continue;
        QStringList strList = lineStr.split(',');

        if (!flag) {
            if (lineStr.contains(STR("证券代码"))) {
                flag = true;

                for (int i = 0; i < strList.count(); i++) {
                    if (strList.at(i) == STR("日期"))
                        dataIdx2fileDataIdx[0] = i;
                    if (strList.at(i) == STR("交易类别"))
                        dataIdx2fileDataIdx[1] = i;
                    if (strList.at(i) == STR("证券代码"))
                        dataIdx2fileDataIdx[2] = i;
                    if (strList.at(i) == STR("证券名称"))
                        dataIdx2fileDataIdx[3] = i;
                    if (strList.at(i) == STR("成交价格"))
                        dataIdx2fileDataIdx[4] = i;
                    if (strList.at(i) == STR("成交数量"))
                        dataIdx2fileDataIdx[5] = i;
                }
            }
        } else {
            float buySellFlag = 1.0f;   // Buy 1.0, Sell -1.0;

            myExchangeData tmpExchangeData;
            tmpExchangeData.time                  = QDateTime::fromString(strList.at(dataIdx2fileDataIdx[0]), "yyyyMMdd");
            tmpExchangeData.exchangeType          = strList.at(dataIdx2fileDataIdx[1]).contains(STR("买")) ?
                                                                          STR("证券买入") : STR("证券卖出");
            tmpExchangeData.assetData.assetCode   = assetCodeWithMarket(strList.at(dataIdx2fileDataIdx[2]));
            tmpExchangeData.assetData.assetName   = strList.at(dataIdx2fileDataIdx[3]);
            tmpExchangeData.assetData.price       = strList.at(dataIdx2fileDataIdx[4]).toDouble();
            tmpExchangeData.assetData.amount      = buySellFlag * qAbs(strList.at(dataIdx2fileDataIdx[5]).toDouble());
            buySellFlag                           = strList.at(dataIdx2fileDataIdx[1]).contains(STR("买")) ? 1.0f : -1.0f;

            tmpExchangeData.accountMoney          = account->dbAccountData.accountData.code;
            tmpExchangeData.assetData.accountCode = tmpExchangeData.accountMoney;
            tmpExchangeData.fee                   = myExchangeFormStock::getStockExchangeFee(tmpExchangeData.assetData.assetCode, tmpExchangeData.assetData.amount,
                                                                                             tmpExchangeData.assetData.price, account->dbAccountData.accountData.note.toDouble());
            tmpExchangeData.money                 = -tmpExchangeData.assetData.price*tmpExchangeData.assetData.amount - tmpExchangeData.fee;
            tmpExchangeData.assetData.type        = STR("股票");

            exchangeDataList.append(tmpExchangeData);
        }
    }
    file.close();
    return true;
}
void myFinanceTreeVeiwContextMenu::undoExchangeList_clicked() {
    int deleteCount = lastExchangeList.count();
    QString info = STR("批量删除上次添加交易数据%1项").arg(deleteCount);
    qDebug() << info;
    for (int i = 0; i < deleteCount; i++) {
        myExchangeData exchangeData;
        exchangeData.id = lastExchangeList.at(i);
        parent->exchangeModel->doExchange(exchangeData, true, i == deleteCount-1);
        qDebug() << "delete exchangeData.id" << exchangeData.id;
    }

    lastExchangeList.clear();
}

QString myFinanceTreeVeiwContextMenu::assetCodeWithMarket(const QString &assetCode) {
    QString ans = assetCode;
    int len = ans.length();
    if (6 != len) {
        int diff = 6 - len;
        QString left = QString(diff, '0');
        ans.insert(0, left);
    }
    QString subStr = ans.left(2);
    QString market;
    if (subStr == "30" || subStr == "00") {
        market = "sz.";
    } else if (subStr == "60") {
        market = "sh.";
    } else {}
    return ans.insert(0, market);
}

void myFinanceTreeVeiwContextMenu::upAsset_clicked() {
    doUpDown(true);
}
void myFinanceTreeVeiwContextMenu::downAsset_clicked() {
    doUpDown(false);
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

/////////////////////////////////////////////////////////////////////////////////
/// 具体实现部分
/////////////////////////////////////////////////////////////////////////////////
void myFinanceTreeVeiwContextMenu::doExchangeStock(const QString &type) {
    myExchangeData exchangeData;
    const myAssetNodeData &holds = GET_CONST_ASSET_NODE_DATA(currentNode);
    exchangeData.accountMoney          = holds.assetData.accountCode;
    exchangeData.assetData.accountCode = holds.assetData.accountCode;
    exchangeData.assetData.assetCode   = holds.assetData.assetCode;
    exchangeData.assetData.assetName   = holds.assetData.assetName;
    exchangeData.exchangeType          = type;
    parent->doExchange(myExchangeUI(exchangeData, false, false), true);
}

void myFinanceTreeVeiwContextMenu::transferIn_clicked() {
    qDebug() << STR("右键转入 clicked");
    myExchangeData exchangeData;
    const myAssetNodeData &holds = GET_CONST_ASSET_NODE_DATA(currentNode);
    exchangeData.assetData.accountCode = holds.assetData.accountCode;
    exchangeData.exchangeType = STR("转帐");
    parent->doExchange(myExchangeUI(exchangeData, false, false), true);
}
void myFinanceTreeVeiwContextMenu::transferOut_clicked() {
    qDebug() << STR("右键转出 clicked");
    myExchangeData exchangeData;
    const myAssetNodeData &holds = GET_CONST_ASSET_NODE_DATA(currentNode);
    exchangeData.accountMoney = holds.assetData.accountCode;
    exchangeData.exchangeType = STR("转帐");
    parent->doExchange(myExchangeUI(exchangeData, false, false), true);
}

void myFinanceTreeVeiwContextMenu::doChangeAssetDirectly(changeType type) {
    qDebug() << STR("myFinanceTreeVeiwContextMenu::doChangeAssetDirectly with type=") << (int)type;
    QString info;
    void *data = nullptr;

    if (myAssetNode::nodeAccount == currentNode->type) {
        const myAccountNodeData &accountInfo = GET_CONST_ACCOUNT_NODE_DATA(currentNode);
        /// INSERT ASSET
        if (POP_INSERT == type) {
            info = STR("添加资产");
            myInsertModifyAsset dial(accountInfo.accountData.code, accountInfo.accountData.name, parent);
            dial.setWindowTitle(info);
            if(dial.exec() == QDialog::Accepted) {
                qDebug() << info + "Accepted";
                myAssetData targetAssetData = dial.getData();
                data = new myAssetData(targetAssetData);
            } else {
                return;
            }
        /// MODIFY ACCOUNT
        } else if (POP_MODIFY == type) {
            info = STR("更新帐户");
            myAccountData originAccountData(accountInfo);
            myInsertModifyAccount dial(parent);
            dial.setWindowTitle(info);
            dial.setUI(myAccountData(accountInfo));
            if(dial.exec() == QDialog::Accepted) {
                qDebug() << info + "Accepted";
                myAccountData targetAccountData = dial.getData();
                if (myAccountData::isSameAccountData(targetAccountData, originAccountData)) {
                    qDebug() << info + "Nothing Changed";
                    return;
                } else {
                    data = new myAccountData(targetAccountData);
                }
            } else {
                return;
            }
        /// DELETE ACCOUNT
        } else if (POP_DELETE == type) {
            info = STR("删除帐户");
            if(QMessageBox::Ok == QMessageBox::warning(parent, info, info + "->\n" +
                                  accountInfo.accountData.code + "\n" + accountInfo.accountData.name,
                                  QMessageBox::Ok|QMessageBox::Cancel, QMessageBox::Ok)) {
                qDebug() << info + "Accepted";
            } else {
                return;
            }
        } else {}
    } else if (myAssetNode::nodeHolds == currentNode->type) {
        const myAssetNodeData   &assetHold   = GET_CONST_ASSET_NODE_DATA(currentNode);
        const myAccountNodeData &accountInfo = GET_ACCOUNT_NODE_DATA(currentNode->parent);
        myInsertModifyAsset dial(accountInfo.accountData.code, accountInfo.accountData.name, parent);
        dial.setUI(myAssetData(assetHold));
        /// MODIFY ASSET
        if (POP_MODIFY == type) {
            info = STR("更新资产");
            dial.setWindowTitle(info);
            if(dial.exec() == QDialog::Accepted) {
                qDebug() << info + "Accepted";
                myAssetData originAssetData(assetHold);
                myAssetData targetAssetData = dial.getData();
                if (myAssetData::isSameAssetData(targetAssetData, originAssetData)) {
                    qDebug() << info + "Nothing Changed";
                    return;
                } else {
                    data = new myAssetData(targetAssetData);
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

    parent->doChangeAssetDirectly(currentNode, type, data, info);
    if (data) {
        delete data; data = nullptr;
    }
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
void myFinanceTreeVeiwContextMenu::stockBonus_intrests(bool isInterest) {
    const myAssetNodeData &holds = GET_CONST_ASSET_NODE_DATA(currentNode);
    myDividendsDialog dial(holds.assetData, isInterest, parent);
    if(dial.exec() == QDialog::Accepted) {
        myDividends dividendsData = dial.getDividendsData();
        qDebug() << dividendsData.toString();
        parent->doDividend(dividendsData, holds.assetData, isInterest);
    }
}
