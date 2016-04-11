#include "myFinanceExchangeWindow.h"
#include "ui_myFinanceExchangeWindow.h"

#include <QtCore/QMap>
#include <QtWidgets/QCheckBox>
#include "myFinanceMainWindow.h"

#include <QtDebug>

myFinanceExchangeWindow::myFinanceExchangeWindow(QWidget *parent) :
    QDialog(parent), grpBuySell(nullptr),
    ui(new Ui::myFinanceExchangeWindow),
    stockCode(myStockCodeName::getInstance()),
    isRollback(false), commisionRate(0.0f),
    remainMoney(0.0f), totalMoney(0.0f),
    remainMoneyOut(0.0f), totalMoneyOut(0.0f),
    remainMoneyIn(0.0f), totalMoneyIn(0.0f)
{
    ui->setupUi(this);

    grpBuySell = new QButtonGroup(this);
    grpBuySell->addButton(ui->radioBuy);
    grpBuySell->addButton(ui->radioSell);
    grpBuySell->setExclusive(true);         //设为互斥
    grpBuySell->setId(ui->radioBuy,  BUY);  //radioBuy的Id设为0
    grpBuySell->setId(ui->radioSell, SELL); //radioBuy的Id设为1
    ui->radioBuy->setChecked(true);
    // updateExchangeType(); 下面有更新

    grpMarket = new QButtonGroup(this);
    grpMarket->addButton(ui->radioSH);
    grpMarket->addButton(ui->radioSZ);
    grpMarket->addButton(ui->radioOther);
    grpMarket->setExclusive(true);           //设为互斥
    grpMarket->setId(ui->radioSH, SH);       //radioBuy的Id设为0
    grpMarket->setId(ui->radioSZ, SZ);       //radioBuy的Id设为1
    grpMarket->setId(ui->radioOther, OTHER); //radioBuy的Id设为2
    //ui->radioSH->setChecked(true);
    ui->radioOther->setChecked(true);
    updateMarketInfo();

    grpIncomeType = new QButtonGroup(this);
    grpIncomeType->addButton(ui->radioSalary);
    grpIncomeType->addButton(ui->radioOtherIncome);
    grpIncomeType->setExclusive(true);              //设为互斥
    grpIncomeType->setId(ui->radioSalary, 0);       //radioSalary的Id设为0
    grpIncomeType->setId(ui->radioOtherIncome, 1);  //radioOtherIncome的Id设为1
    ui->radioSalary->setChecked(true);
    ui->lineEditIncomeType->setDisabled(true);

    ui->lineEditExpendCode->setText(QString::fromLocal8Bit("类别"));
    ui->lineEditExpendName->setText(QString::fromLocal8Bit("名称"));

    dataSource = 0;
    ui->tabWidget->setCurrentIndex(dataSource);
    updataData();
    updateBuySell();

    initial(static_cast<myFinanceMainWindow *>(parent)->getAssetModel()->getRootNode());

    updateExchangeFee();

    ui->checkBoxRollback->setChecked(isRollback);
    ui->checkBoxRollback->hide();
}

myFinanceExchangeWindow::~myFinanceExchangeWindow()
{
    if (!grpIncomeType)
        delete grpIncomeType;
    if (!grpMarket)
        delete grpMarket;
    if (!grpBuySell)
        delete grpBuySell;
    delete ui;
}

void myFinanceExchangeWindow::initial(const myRootAccountAsset &rootNode) {
    this->rootNode = &rootNode;

    ui->moneyAccount->clear();
    // 交易：资产变化
    for (int i = 0; i < rootNode.getAccountCount(); i++) {
        myAssetNode *accountNode = rootNode.getAccountNode(i);
        const myAssetAccount &accountData = accountNode->nodeData.value<myAssetAccount>();
        QIcon   icon = QString(":/icon/finance/resource/icon/finance/%1").arg(accountData.logo);
        QString code;
        if (accountData.name.contains(QString::fromLocal8Bit("银行"))) {
            code = "**** **** " + accountData.code.right(4);
        } else {
            code = accountData.code;
        }
        ui->moneyAccount->addItem(icon, code);
    }
    // 转帐：资金变化
    for (int i = 0; i < rootNode.getAccountCount(); i++) {
        myAssetNode *accountNode = rootNode.getAccountNode(i);
        for (int j = 0; j < accountNode->children.count(); j++) {
            myAssetNode *holdNode = accountNode->children.at(j);
            QString assetCode = holdNode->nodeData.value<myAssetHold>().assetCode;
            if (assetCode.contains("cash")) {
                const myAssetAccount &accountData = accountNode->nodeData.value<myAssetAccount>();
                QIcon   icon = QString(":/icon/finance/resource/icon/finance/%1").arg(accountData.logo);
                QString code;
                if (accountData.name.contains(QString::fromLocal8Bit("银行"))) {
                    code = "**** **** " + accountData.code.right(4);
                } else {
                    code = accountData.code;
                }
                ui->moneyAccountOut->addItem(icon, code);
                ui->moneyAccountIn->addItem(icon, code);
                break;
            }
        }
    }
    // 收入，支出
    for (int i = 0; i < rootNode.getAccountCount(); i++) {
        myAssetNode *accountNode = rootNode.getAccountNode(i);
        if (accountNode->nodeData.value<myAssetAccount>().type.contains(QString::fromLocal8Bit("券商"))) {
                continue;
        }
        for (int j = 0; j < accountNode->children.count(); j++) {
            myAssetNode *holdNode = accountNode->children.at(j);
            QString assetCode = holdNode->nodeData.value<myAssetHold>().assetCode;
            if (assetCode.contains("cash")) {
                const myAssetAccount &accountData = accountNode->nodeData.value<myAssetAccount>();
                QIcon   icon = QString(":/icon/finance/resource/icon/finance/%1").arg(accountData.logo);
                QString code;
                if (accountData.name.contains(QString::fromLocal8Bit("银行"))) {
                    code = "**** **** " + accountData.code.right(4);
                } else {
                    code = accountData.code;
                }
                ui->moneyAccountIncome->addItem(icon, code);
                ui->moneyAccountExpend->addItem(icon, code);
                break;
            }
        }
    }
}

void myFinanceExchangeWindow::on_buttonBox_accepted()
{
    qDebug() << QString::fromLocal8Bit("资产变化 accepted");
    data.time     = ui->timeDateTimeEdit->dateTime();
    data.type     = ui->typeLineEdit->text();
    updataData();
}

void myFinanceExchangeWindow::on_spinBoxAmount_valueChanged(int value)
{
    data.amount = value;
    updateBuySell();
}
void myFinanceExchangeWindow::on_spinBoxPrice_valueChanged(double value) {
    data.price = value;
    updateBuySell();
}
void myFinanceExchangeWindow::on_radioBuy_clicked() {
    qDebug() << "radioBuy clicked";
    updateBuySell();
    updateExchangeType();
}
void myFinanceExchangeWindow::on_radioSell_clicked() {
    qDebug() << "radioSell clicked";
    updateBuySell();
    updateExchangeType();
}
void myFinanceExchangeWindow::on_exchangeFeeSpinBox_valueChanged(double value) {
    qDebug() << "exchangeFeeSpinBox valueChanged";
    data.fee = value;
    updateBuySell();
}
void myFinanceExchangeWindow::updateBuySell() {
    data.buySell = grpBuySell->checkedId() == BUY ? static_cast<bool>(BUY) : static_cast<bool>(SELL);
    buySellFlag = data.buySell == SELL ? 1.0f : -1.0f;

    data.amount = -buySellFlag*qAbs(data.amount);
    data.money = -static_cast<float>(data.amount) * data.price - data.fee;

    ui->moneySpinBox->setValue(data.money);

    if (grpMarket->checkedId() != OTHER) {
        updateExchangeFee();
    }

    qDebug() << "data.buySell " << (grpBuySell->checkedId() == BUY ? "BUY" : "SELL") << ","
             << "data.amount " << data.amount << ","
             << "data.money "  << data.money  << ",";
}

void myFinanceExchangeWindow::on_tabWidget_currentChanged(int index)
{
    dataSource = index;
    qDebug() << dataSource;
    updataData();
    updateExchangeFee();
}

/// 1. 构造时调用
/// 2. tab改变后调用
/// 3. accepted
void myFinanceExchangeWindow::updataData() {
    if (0 == dataSource) {
        data.account1 = ui->moneyAccount->itemText(ui->moneyAccount->currentIndex());
        data.money  = ui->moneySpinBox->value();

        data.account2 = data.account1;
        data.code   = ui->codeLineEdit->text();
        if (data.code == "cash") {
            data.amount = 1;
        } else {
            data.amount = ui->spinBoxAmount->text().toInt() * -buySellFlag;
        }
        data.price  = ui->spinBoxPrice->text().toDouble();
        data.name   = ui->lineEditName->text();
} else if (1 == dataSource) {
        data.account1 = ui->moneyAccountOut->itemText(ui->moneyAccountOut->currentIndex());
        data.money    = -ui->moneyTransferSpinBox->value();

        data.account2 = ui->moneyAccountIn->itemText(ui->moneyAccountIn->currentIndex());
        data.code     = MY_CASH;
        data.name     = QString::fromLocal8Bit("现有资金");
        data.amount   = 1;
        data.price    = ui->moneyTransferSpinBox->value();
    } else if (2 == dataSource) {
        data.account2 = ui->moneyAccountIncome->itemText(ui->moneyAccountIncome->currentIndex());
        data.code     = MY_CASH;
        if (grpIncomeType->checkedId() == 0) {
            data.name = QString::fromLocal8Bit("工资收入");
        } else if (grpIncomeType->checkedId() == 1) {
            data.name = ui->lineEditIncomeType->text();
        } else {}
        data.amount   = 1;
        data.price    = ui->spinBoxIncome->value();

        data.account1 = OTHER_ACCOUNT;
        data.money    = -data.price;
    } else if (3 == dataSource) {
        data.account1 = ui->moneyAccountExpend->itemText(ui->moneyAccountExpend->currentIndex());
        data.money    = -ui->spinBoxExpend->value();

        data.account2 = OTHER_ACCOUNT;
        data.code     = ui->lineEditExpendCode->text();
        data.name     = ui->lineEditExpendName->text();
        data.amount   = 1;
        data.price    = ui->moneyTransferSpinBox->value();
    } else {}
    data.fee    = ui->exchangeFeeSpinBox->value();
    updateExchangeType();   //负责data.type部分的更新和显示

    qDebug() << "data.time "     << data.time << ","
             << "data.type "     << data.type << ","
             << "data.account1 " << data.account1 << ","
             << "data.money "    << data.money;
    qDebug() << "data.account2 " << data.account2 << ","
             << "data.code "     << data.code << ","
             << "data.name "     << data.name << ","
             << "data.amount "   << data.amount << ","
             << "data.price "    << data.price;
}

void myFinanceExchangeWindow::on_radioSH_clicked() {
    qDebug() << "radioSH clicked";
    updateMarketInfo();
}
void myFinanceExchangeWindow::on_radioSZ_clicked() {
    qDebug() << "radioSZ clicked";
    updateMarketInfo();
}
void myFinanceExchangeWindow::on_radioOther_clicked() {
    qDebug() << "radioOther clicked";
    updateMarketInfo();
}

void myFinanceExchangeWindow::updateMarketInfo() {
    QString market;
    switch (grpMarket->checkedId()) {
    case SH:
        market = "sh.";
        break;
    case SZ:
        market = "sz.";
        break;
    case OTHER:
        market = "";
        break;
    default:
        break;
    }
    int pointIndex = data.code.indexOf(QString("."));
    data.code.remove(0, pointIndex+1);
    data.code.insert(0, market);
    if (data.code == "sh.sh") {
        data.code = "sh.";
    } else if (data.code == "sz.sz") {
        data.code = "sz.";
    } else {}
    ui->codeLineEdit->setText(data.code);

    updateExchangeFee();

    qDebug() << "updateMarketInfo()" << ","
             << "data.code"  << data.code << ","
             << (grpMarket->checkedId() == SH ? "radioSH" :
                 grpMarket->checkedId() == SZ ? "radioSZ" :
                 grpMarket->checkedId() == OTHER ? "radioOther" : "radioUnknown");
}

void myFinanceExchangeWindow::on_codeLineEdit_textChanged(const QString &str)
{
    data.code = str;

    // 上海，深圳通过股票代码自动判断
    int pointIndex = data.code.indexOf(QString("."));
    int len = data.code.size();
    if (len - pointIndex > 2 &&
        (grpMarket->checkedId() == SH ||
         grpMarket->checkedId() == SZ)) {
        QString subStr = data.code.mid(pointIndex+1, 2);
        if (subStr == "30" || subStr == "00") {
            ui->radioSZ->setChecked(true);
        } else if (subStr == "60") {
            ui->radioSH->setChecked(true);
        } else {
            subStr = data.code.mid(pointIndex+1, 4);
            if (subStr == "cash") {
                ui->radioOther->setChecked(true);
            } else {}
        }
    } else if (grpMarket->checkedId() == OTHER) {
        QString subStr = data.code.left(3);
        if (subStr == "sh.") {
            ui->radioSH->setChecked(true);
        } else if (subStr == "sz.") {
            ui->radioSZ->setChecked(true);
        } else {}
    }

    updateMarketInfo();

    if (data.code == "cash") {
        data.amount = 1;
        ui->spinBoxAmount->setValue(data.amount);
        ui->spinBoxAmount->setDisabled(true);
        ui->labelPrice->setText(QString::fromLocal8Bit("资金："));
    } else {
        if (!ui->spinBoxAmount->isEnabled()) {
            ui->spinBoxAmount->setEnabled(true);
            ui->labelPrice->setText(QString::fromLocal8Bit("单价："));
        }
    }
}
void myFinanceExchangeWindow::on_codeLineEdit_editingFinished()
{
    int count = stockCode->codeName.count();
    qDebug() << QString::fromLocal8Bit("代号EditLine") << ui->codeLineEdit->text() << "(" << count << ")";
    if (OTHER != grpMarket->checkedId()) {
        if (stockCode->getIsInitialed()) {
            data.name = stockCode->findNameFromCode(data.code);
            ui->lineEditName->setText(data.name);
        }
    }
}
void myFinanceExchangeWindow::on_nameLineEdit_editingFinished()
{
    QString str = ui->lineEditName->text();
    int count = stockCode->codeName.count();
    qDebug() << QString::fromLocal8Bit("名称EditLine") << str << "(" << count << ")";

    QMap<QString,QString>::const_iterator it = stockCode->codeName.begin();
    for (; it != stockCode->codeName.end(); ++it) {
        if (it.value() == str) {
            data.code = it.key();
            QRegExp rx("([a-zA-Z]*)[.][0-9]*");
            int pos = data.code.indexOf(rx);
            if (pos >= 0) {
                QString a = rx.cap(1);
                if (a == "sh") {
                    ui->radioSH->setChecked(true);
                } else if (a == "sz") {
                    ui->radioSZ->setChecked(true);
                } else {
                    ui->radioOther->setChecked(true);
                }
                updateMarketInfo();
            }

            ui->codeLineEdit->setText(data.code);
            qDebug() << data.code << it.value();
            return;
        }
    }

    ui->radioOther->setChecked(true);
    updateMarketInfo();
}

void myFinanceExchangeWindow::on_feeRateSpinBox_valueChanged(double feeRate)
{
    qDebug() << QString::fromLocal8Bit("佣金") << feeRate;
    commisionRate = feeRate * 0.001f;
    updateExchangeFee();
}
void myFinanceExchangeWindow::updateExchangeFee() {
    double fee = 0.0f;
    double amount = qAbs(static_cast<double>(data.amount));

    double fee1 = 0.0f; //佣金
    double fee2 = 0.0f; //过户费
    double fee3 = 0.0f; //印花税

    if (ui->tabWidget->currentIndex() == 0) {
        if (OTHER != grpMarket->checkedId()) {
            fee1 = data.price * amount * commisionRate;
            if (fee1 < 5.0f) {
                fee1 = 5.0f;
            } else {}
        }

        if (SH == grpMarket->checkedId()) {
            fee2 = data.price * amount * 0.02f*0.001f;  //0.02‰
        } else {}

        if (SELL == grpBuySell->checkedId()) {
            fee3 = data.price * amount * 0.001f;
        } else {}
    }
    fee = fee1 + fee2 + fee3;
    data.fee = fee;
    ui->exchangeFeeSpinBox->setValue(fee);
}

/// 1. updataData
/// 2. radioBuy/Sell变化时
/// 3. lineEditIncomeType or radioSalary/Other Changed
void myFinanceExchangeWindow::updateExchangeType() {
    if (0 == dataSource) {
        if (grpBuySell->checkedId() == BUY) {
            data.type = QString::fromLocal8Bit("证券买入");
        } else {
            data.type = QString::fromLocal8Bit("证券卖出");
        }
    } else if (1 == dataSource) {
        data.type = QString::fromLocal8Bit("转帐");
    } else if (2 == dataSource) {
        data.type = QString::fromLocal8Bit("收入");
        updateIncomeType();
    } else if (3 == dataSource) {
        data.type = QString::fromLocal8Bit("支出");
    } else {}

    ui->typeLineEdit->setText(data.type);
}

void myFinanceExchangeWindow::on_radioSalary_clicked() {
    qDebug() << "radioSalary clicked";
    updateExchangeType();
}
void myFinanceExchangeWindow::on_radioOtherIncome_clicked() {
    qDebug() << "radioOtherIncome clicked";
    updateExchangeType();
}
void myFinanceExchangeWindow::on_lineEditIncomeType_textChanged(const QString &str) {
    data.name = str;
}
void myFinanceExchangeWindow::updateIncomeType() {
    if (grpIncomeType->checkedId() == 0) {
        data.name = QString::fromLocal8Bit("工资收入");
        ui->lineEditIncomeType->setDisabled(true);
    } else if (grpIncomeType->checkedId() == 1) {
        ui->lineEditIncomeType->setEnabled(true);
        data.name = ui->lineEditIncomeType->text();
    } else {}
}

void myFinanceExchangeWindow::showRollback() {
    ui->checkBoxRollback->setVisible(true);
}
void myFinanceExchangeWindow::setUI(const myExchangeData &exchangeData, bool rollbackShow) {
    data = exchangeData;
    ui->timeDateTimeEdit->setDateTime(data.time);
    ui->typeLineEdit->setText(data.type);
    if (qAbs(data.fee) > MONEY_EPS) {
        ui->exchangeFeeSpinBox->setValue(data.fee);
    } else {
        double fee = static_cast<double>(data.amount)*data.price + data.money;
        ui->exchangeFeeSpinBox->setValue(fee);
    }
    if (rollbackShow) {
        showRollback();
    }

    if (data.type.contains(QString::fromLocal8Bit("转帐"))) {
        ui->tabWidget->setCurrentIndex(1);

        int indexOut = ui->moneyAccountOut->findText(data.account1);
        ui->moneyAccountOut->setCurrentIndex(indexOut);
        int indexIn = ui->moneyAccountIn->findText(data.account2);
        ui->moneyAccountIn->setCurrentIndex(indexIn);

        ui->moneyTransferSpinBox->setValue(data.price);
    } else if (data.type.contains(QString::fromLocal8Bit("证券"))) {
        ui->tabWidget->setCurrentIndex(0);

        int index = ui->moneyAccount->findText(data.account2);
        ui->moneyAccount->setCurrentIndex(index);
        ui->lineEditName->setText(data.name);
        ui->codeLineEdit->setText(data.code);
        ui->spinBoxPrice->setValue(data.price);
        ui->spinBoxAmount->setValue(data.amount);
    } else if (data.type.contains(QString::fromLocal8Bit("收入"))) {
    } else if (data.type.contains(QString::fromLocal8Bit("支出"))) {
    } else {}
}

void myFinanceExchangeWindow::on_checkBoxRollback_clicked() {
    isRollback = (Qt::Checked == ui->checkBoxRollback->checkState());
}
void myFinanceExchangeWindow::on_moneySpinBox_valueChanged(double value) {
    data.money = value;
    remainMoney = totalMoney + data.money;
    ui->moneySpinBoxRemain->setValue(remainMoney);
}
void myFinanceExchangeWindow::on_moneyAccount_currentIndexChanged(int index) {
    totalMoney = getTotalMoney(index);
    ui->moneySpinBoxTotal->setValue(totalMoney);
}

void myFinanceExchangeWindow::on_moneyAccountOut_currentIndexChanged(int index) {
    totalMoneyOut = getTotalMoney(index);
    ui->moneySpinBoxTotalOut->setValue(totalMoneyOut);
}

void myFinanceExchangeWindow::on_moneyAccountIn_currentIndexChanged(int index) {
    totalMoneyIn = getTotalMoney(index);
    ui->moneySpinBoxTotalIn->setValue(totalMoneyIn);
}
float myFinanceExchangeWindow::getTotalMoney(int index) {
    float tmpTotalMoney = 0.0f;
    myAssetNode *accountNode = rootNode->getAccountNode(index);
    int numAsset = accountNode->children.size();
    for (int i = 0; i < numAsset; i++) {
        myAssetNode *asset = accountNode->children.at(i);
        const myAssetHold &assetHold = (asset->nodeData).value<myAssetHold>();
        if (assetHold.assetCode == MY_CASH) {
            qDebug() << assetHold.accountCode << " " << assetHold.assetCode << " " << assetHold.price;
            tmpTotalMoney = assetHold.price;
            break;
        }
    }
    return tmpTotalMoney;
}
void myFinanceExchangeWindow::on_moneyTransferSpinBox_valueChanged(double value) {
    data.price = value;
    data.money = -data.price;

    remainMoneyOut = totalMoneyOut + data.money;
    ui->moneySpinBoxRemainOut->setValue(remainMoneyOut);
    remainMoneyIn = totalMoneyIn - data.money;
    ui->moneySpinBoxRemainIn->setValue(remainMoneyIn);
}
