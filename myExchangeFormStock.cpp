#include "myExchangeFormStock.h"
#include "ui_myExchangeFormStock.h"

myExchangeFormStock::myExchangeFormStock(const myRootAccountAsset *rootNode, QString tabName, QWidget *parent) :
    myExchangeFormTabBase(rootNode, tabName, myExchangeUI::TAB_STOCK, parent),
    stockCode(myStockCodeName::getInstance()),
    totalMoney(0.0f), remainMoney(0.0f),
    ui(new Ui::myExchangeFormStock)
{
    ui->setupUi(this);

    grpBuySell = new QButtonGroup(this);
    grpBuySell->addButton(ui->radioBuy);
    grpBuySell->addButton(ui->radioSell);
    grpBuySell->setExclusive(true);         //设为互斥
    grpBuySell->setId(ui->radioBuy,  BUY);  //radioBuy的Id设为0
    grpBuySell->setId(ui->radioSell, SELL); //radioBuy的Id设为1
    ui->radioBuy->setChecked(true);

    grpMarket = new QButtonGroup(this);
    grpMarket->addButton(ui->radioSH);
    grpMarket->addButton(ui->radioSZ);
    grpMarket->addButton(ui->radioOther);
    grpMarket->setExclusive(true);           //设为互斥
    grpMarket->setId(ui->radioSH, SH);       //radioBuy的Id设为0
    grpMarket->setId(ui->radioSZ, SZ);       //radioBuy的Id设为1
    grpMarket->setId(ui->radioOther, OTHER); //radioBuy的Id设为2
    ui->radioOther->setChecked(true);
    on_radioBuy_clicked();

    updateMarketInfo();
    updateBuySell();
    updateExchangeFee();

    // 控件ComboBox -> ACCOUNT
    exchangeIdx2AccountIdx.clear();
    int localCount = 0;
    for (int i = 0; i < rootNode->getAccountCount(); i++) {
        myAssetNode *accountNode = rootNode->getAccountNode(i);
        const myAssetAccount &accountData = accountNode->nodeData.value<myAssetAccount>();
        if (!accountData.type.contains(STR("券商")))
            continue;

        QIcon   icon = QIcon(QString(":/icon/finance/resource/icon/finance/%1").arg(accountData.logo));
        QString code;
        if (accountData.name.contains(STR("银行"))) {
            code = "**** **** " + accountData.code.right(4);
        } else {
            code = accountData.code;
        }
        exchangeIdx2AccountIdx.insert(localCount, i);
        ui->moneyAccount->addItem(icon, code);
        localCount++;
    }
}

myExchangeFormStock::~myExchangeFormStock() {
    delete ui;
}

void myExchangeFormStock::recordExchangeData(myExchangeData &tmpData) {
    myExchangeFormTabBase::recordExchangeData(tmpData);

    tmpData.account1 = ui->moneyAccount->itemText(ui->moneyAccount->currentIndex());
    tmpData.money  = ui->moneySpinBox->value();

    tmpData.account2 = data.account1;
    tmpData.code   = ui->codeLineEdit->text();
    if (data.code == "cash") {
        tmpData.amount = 1;
    } else {
        tmpData.amount = ui->spinBoxAmount->text().toInt() * -buySellFlag;
    }
    tmpData.price  = ui->spinBoxPrice->text().toDouble();
    tmpData.name   = ui->lineEditName->text();
}
void myExchangeFormStock::setUI(const myExchangeData &exchangeData) {
    int index = ui->moneyAccount->findText(exchangeData.account2);
    ui->moneyAccount->setCurrentIndex(exchangeIdx2AccountIdx.find(index).value());
    ui->lineEditName->setText(exchangeData.name);
    ui->codeLineEdit->setText(exchangeData.code);
    ui->spinBoxPrice->setValue(exchangeData.price);
    ui->spinBoxAmount->setValue(exchangeData.amount);

    myExchangeFormTabBase::setUI(exchangeData);
}

void myExchangeFormStock::updateBuySell() {
    buySellFlag = grpBuySell->checkedId() == SELL ? 1.0f : -1.0f;
    data.amount = -buySellFlag*qAbs(data.amount);

    // market, price, amount, fee 决定 money
    // market, price*amount, buy/sell 决定 fee
    if (grpMarket->checkedId() != OTHER) {
        updateExchangeFee();
    }
    data.money = -static_cast<float>(data.amount) * data.price - data.fee;
    ui->moneySpinBox->setValue(data.money);

    qDebug() << "#updateBuySell# data.buySell " << (grpBuySell->checkedId() == BUY ? "BUY" : "SELL") << ","
             << "data.amount " << data.amount << ","
             << "data.money "  << data.money  << ",";
}

void myExchangeFormStock::updateMarketInfo() {
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
    if (stockCode->getIsInitialed()) {
        data.name = stockCode->findNameFromCode(data.code);
        ui->lineEditName->setText(data.name);
    }
    qDebug() << "updateMarketInfo()" << ","
             << "data.code"  << data.code << ","
             << (grpMarket->checkedId() == SH ? "radioSH" :
                 grpMarket->checkedId() == SZ ? "radioSZ" :
                 grpMarket->checkedId() == OTHER ? "radioOther" : "radioUnknown");
}

void myExchangeFormStock::updateExchangeFee() {
    double fee = 0.0f;
    double amount = qAbs(static_cast<double>(data.amount));

    double fee1 = 0.0f; //佣金
    double fee2 = 0.0f; //过户费
    double fee3 = 0.0f; //印花税

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

    fee = fee1 + fee2 + fee3;
    data.fee = fee;
    setExchangeWindowFee(data.fee);
}

void myExchangeFormStock::on_codeLineEdit_textChanged(const QString &str) {
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
        ui->labelPrice->setText(STR("资金："));
    } else {
        if (!ui->spinBoxAmount->isEnabled()) {
            ui->spinBoxAmount->setEnabled(true);
            ui->labelPrice->setText(STR("单价："));
        }
    }
}
void myExchangeFormStock::on_codeLineEdit_editingFinished() {
    int count = stockCode->codeName.count();
    qDebug() << STR("代号EditLine") << ui->codeLineEdit->text() << "(" << count << ")";
    if (OTHER != grpMarket->checkedId()) {
        if (stockCode->getIsInitialed()) {
            data.name = stockCode->findNameFromCode(data.code);
            ui->lineEditName->setText(data.name);
        }
    }
}
void myExchangeFormStock::on_nameLineEdit_editingFinished() {
    QString str = ui->lineEditName->text();
    int count = stockCode->codeName.count();
    qDebug() << STR("名称EditLine") << str << "(" << count << ")";

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

void myExchangeFormStock::on_spinBoxAmount_valueChanged(int value) {
    data.amount = buySellFlag*qAbs(value);
    updateBuySell();
}
void myExchangeFormStock::on_spinBoxPrice_valueChanged(double value) {
    data.price = value;
    updateBuySell();
}

void myExchangeFormStock::on_radioBuy_clicked() {
    qDebug() << "#radioBuy_clicked#";
    updateBuySell();
    data.type = STR("证券买入");
    setExchangeWindowType(data.type);
}
void myExchangeFormStock::on_radioSell_clicked() {
    qDebug() << "#radioSell_clicked#";
    updateBuySell();
    data.type = STR("证券卖出");
    setExchangeWindowType(data.type);
}

void myExchangeFormStock::on_radioSH_clicked() {
    qDebug() << "#radioSH_clicked#";
    updateMarketInfo();
}
void myExchangeFormStock::on_radioSZ_clicked() {
    qDebug() << "#radioSZ_clicked#";
    updateMarketInfo();
}
void myExchangeFormStock::on_radioOther_clicked() {
    qDebug() << "#radioOther_clicked#";
    updateMarketInfo();
}

void myExchangeFormStock::on_moneyAccount_currentIndexChanged(int index) {
    int nodeIdx = exchangeIdx2AccountIdx.find(index).value();
    // 1. data.account1 & data.account2 update
    myAssetNode *accountNode = rootNode->getAccountNode(nodeIdx);
    const myAssetAccount accountData = accountNode->nodeData.value<myAssetAccount>();
    data.account1 = accountData.code;
    data.account2 = accountData.code;
    // 2. totalMoney
    totalMoney = getTotalMoney(nodeIdx);
    if (isModifyExchange) {
        totalMoney -= usedMoneyBeforeModify;
    }
    ui->moneySpinBoxTotal->setValue(totalMoney);
    qDebug() << "#moneyAccount_currentIndexChanged# isModifyExchange:" << isModifyExchange
             << " totalMoney:" << totalMoney;
    // 3. aommisionRate
    commisionRate = accountNode->nodeData.value<myAssetAccount>().note.toDouble();
    ui->feeRateSpinBox->setValue(commisionRate*1000);
    qDebug() << STR("佣金") << commisionRate;
    // 4. fee & money
    updateExchangeFee();
    data.money = static_cast<float>(data.amount) * data.price - data.fee;
    ui->moneySpinBox->setValue(data.money);
}
void myExchangeFormStock::on_moneySpinBox_valueChanged(double value) {
    data.money = value;
    remainMoney = totalMoney + data.money;
    ui->moneySpinBoxRemain->setValue(remainMoney);
    qDebug() << "#moneySpinBox_valueChanged# remainMoney:" << remainMoney
             << " data.money:" << data.money << " totalMoney:" << totalMoney;
}

void myExchangeFormStock::exchangeWindowFeeChanged(double fee) {
    qDebug() << "$$myExchangeFormStock::exchangeWindowFeeChanged " << fee << "$$";
    myExchangeFormTabBase::exchangeWindowFeeChanged(fee);
    data.money = static_cast<float>(data.amount) * data.price - data.fee;
    ui->moneySpinBox->setValue(data.money);
}
