#include "myExchangeFormStock.h"
#include "ui_myExchangeFormStock.h"
#include "AssetCode2Type.h"

myExchangeFormStock::myExchangeFormStock(const myRootAccountAsset *rootNode, QString tabName, QWidget *parent) :
    myExchangeFormTabBase(rootNode, tabName, myExchangeUI::TAB_STOCK, parent),
    stockCode(myStockCodeName::getInstance()),
    totalMoney(0.0f), remainMoney(0.0f), isShowRemainStock(true),
    accountNode(nullptr),
    ui(new Ui::myExchangeFormStock)
{
    ui->setupUi(this);

    grpBuySell = new QButtonGroup(this);
    grpBuySell->addButton(ui->radioBuy);
    grpBuySell->addButton(ui->radioSell);
    grpBuySell->setExclusive(true);         //��Ϊ����
    grpBuySell->setId(ui->radioBuy,  BUY);  //radioBuy��Id��Ϊ0
    grpBuySell->setId(ui->radioSell, SELL); //radioBuy��Id��Ϊ1

    grpMarket = new QButtonGroup(this);
    grpMarket->addButton(ui->radioSH);
    grpMarket->addButton(ui->radioSZ);
    grpMarket->addButton(ui->radioOther);
    grpMarket->setExclusive(true);           //��Ϊ����
    grpMarket->setId(ui->radioSH, SH);       //radioBuy��Id��Ϊ0
    grpMarket->setId(ui->radioSZ, SZ);       //radioBuy��Id��Ϊ1
    grpMarket->setId(ui->radioOther, OTHER); //radioBuy��Id��Ϊ2

    ui->radioOther->click();
    ui->radioBuy->click();

    updateMarketInfo();
    updateBuySell();
    updateExchangeFee();

    // �ؼ�ComboBox -> ACCOUNT
    exchangeIdx2AccountIdx.clear();
    int localCount = 0;
    for (int i = 0; i < rootNode->getAccountCount(); i++) {
        myAssetNode *accountNode = rootNode->getAccountNode(i);
        const myAssetAccount accountData = accountNode->nodeData.value<myAssetAccount>();
        if (!accountData.accountData.type.contains(STR("ȯ��")))
            continue;

        QIcon   icon = QIcon(QString(":/icon/finance/resource/icon/finance/%1").arg(accountData.logo));
        QString code;
        if (accountData.accountData.name.contains(STR("����"))) {
            code = "**** **** " + accountData.accountData.code.right(4);
        } else {
            code = accountData.accountData.code;
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

    if (accountNode)
        tmpData.accountMoney = accountNode->nodeData.value<myAssetAccount>().accountData.code;;
    tmpData.money  = ui->moneySpinBox->value();

    tmpData.assetData.accountCode = data.accountMoney;
    tmpData.assetData.assetCode   = ui->codeLineEdit->text();
    if (data.assetData.assetCode == "cash") {
        tmpData.assetData.amount = 1;
    } else {
        tmpData.assetData.amount = ui->spinBoxAmount->text().toInt() * -buySellFlag;
    }
    tmpData.assetData.price     = ui->spinBoxPrice->text().toDouble();
    tmpData.assetData.assetName = ui->nameLineEdit->text();
    tmpData.assetData.type      = data.assetData.type;
}
void myExchangeFormStock::setUI(const myExchangeData &exchangeData) {
    int index = ui->moneyAccount->findText(exchangeData.assetData.accountCode);
    ui->moneyAccount->setCurrentIndex(exchangeIdx2AccountIdx.find(index).value());
    ui->nameLineEdit->setText(exchangeData.assetData.assetCode);
    ui->codeLineEdit->setText(exchangeData.assetData.assetName);
    ui->spinBoxPrice->setValue(exchangeData.assetData.price);
    ui->spinBoxAmount->setValue(qAbs(exchangeData.assetData.amount));
    if (STR("֤ȯ����") == exchangeData.exchangeType) {
        ui->radioBuy->click();
    } else if (STR("֤ȯ����") == exchangeData.exchangeType) {
        ui->radioSell->click();
    } else {}

    myExchangeFormTabBase::setUI(exchangeData);
}
void myExchangeFormStock::checkAndSetDisable(const myExchangeData &exchangeData) {
    setUI(exchangeData);
    if (exchangeData.accountMoney == exchangeData.assetData.accountCode && exchangeData.accountMoney != "")
        ui->moneyAccount->setDisabled(true);
    if (exchangeData.assetData.assetCode != "") {
        ui->codeLineEdit->setDisabled(true);
        ui->radioSH->setDisabled(true);
        ui->radioSZ->setDisabled(true);
        ui->radioOther->setDisabled(true);
    }
    if (exchangeData.assetData.assetName != "")
        ui->nameLineEdit->setDisabled(true);
}

void myExchangeFormStock::updateBuySell() {
    buySellFlag = grpBuySell->checkedId() == SELL ? 1.0f : -1.0f;
    data.assetData.amount = -buySellFlag*qAbs(data.assetData.amount);

    // market, price, amount, fee ���� money
    // market, price*amount, buy/sell ���� fee
    if (grpMarket->checkedId() != OTHER) {
        updateExchangeFee();
    }
    ui->moneySpinBox->setValue(calcMoney());

    qDebug() << "#updateBuySell# data.buySell " << (grpBuySell->checkedId() == BUY ? "BUY" : "SELL") << ","
             << "data.amount " << data.assetData.amount << ","
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
    int pointIndex = data.assetData.assetCode.indexOf(QString("."));
    data.assetData.assetCode.remove(0, pointIndex+1);
    data.assetData.assetCode.insert(0, market);
    if (data.assetData.assetCode == "sh.sh") {
        data.assetData.assetCode = "sh.";
    } else if (data.assetData.assetCode == "sz.sz") {
        data.assetData.assetCode = "sz.";
    } else {}
    ui->codeLineEdit->setText(data.assetData.assetCode);

    updateExchangeFee();
    if (stockCode->getIsInitialed()) {
        data.assetData.assetName = stockCode->findNameFromCode(data.assetData.assetCode);
        ui->nameLineEdit->setText(data.assetData.assetName);
    }
    qDebug() << "updateMarketInfo()" << ","
             << "data.code"  << data.assetData.assetCode << ","
             << "data.name"  << data.assetData.assetName << ","
             << (grpMarket->checkedId() == SH ? "radioSH" :
                 grpMarket->checkedId() == SZ ? "radioSZ" :
                 grpMarket->checkedId() == OTHER ? "radioOther" : "radioUnknown");
}

void myExchangeFormStock::updateExchangeFee() {
    double fee = 0.0f;
    double amount = qAbs(static_cast<double>(data.assetData.amount));

    double fee1 = 0.0f; //Ӷ��
    double fee2 = 0.0f; //������
    double fee3 = 0.0f; //ӡ��˰

    if (OTHER != grpMarket->checkedId()) {
        fee1 = data.assetData.price * amount * commisionRate;
        if (fee1 < 5.0f) {
            fee1 = 5.0f;
        } else {}
    }

    if (SH == grpMarket->checkedId()) {
        fee2 = data.assetData.price * amount * 0.02f*0.001f;  //0.02��
    } else {}

    if (SELL == grpBuySell->checkedId()) {
        fee3 = data.assetData.price * amount * 0.001f;
    } else {}

    fee = fee1 + fee2 + fee3;
    data.fee = static_cast<float>(static_cast<int>(fee*100+0.5))/100;
    setExchangeWindowFee(data.fee);
}

void myExchangeFormStock::on_codeLineEdit_textChanged(const QString &str) {
    data.assetData.assetCode = str;

    // �Ϻ�������ͨ����Ʊ�����Զ��ж�
    int pointIndex = data.assetData.assetCode.indexOf(QString("."));
    int len = data.assetData.assetCode.size();
    if (len - pointIndex > 2 &&
        (grpMarket->checkedId() == SH ||
         grpMarket->checkedId() == SZ)) {
        QString subStr = data.assetData.assetCode.mid(pointIndex+1, 2);
        if (subStr == "30" || subStr == "00") {
            ui->radioSZ->setChecked(true);
        } else if (subStr == "60") {
            ui->radioSH->setChecked(true);
        } else {
            subStr = data.assetData.assetCode.mid(pointIndex+1, 4);
            if (subStr == "cash") {
                ui->radioOther->setChecked(true);
            } else {}
        }
    } else if (grpMarket->checkedId() == OTHER) {
        QString subStr = data.assetData.assetCode.left(3);
        if (subStr == "sh.") {
            ui->radioSH->setChecked(true);
        } else if (subStr == "sz.") {
            ui->radioSZ->setChecked(true);
        } else {}
    }

    updateMarketInfo();

    if (data.assetData.assetCode == "cash") {
        data.assetData.amount = 1;
        ui->spinBoxAmount->setValue(qAbs(data.assetData.amount));
        ui->spinBoxAmount->setDisabled(true);
        ui->labelPrice->setText(STR("�ʽ�"));
    } else {
        if (!ui->spinBoxAmount->isEnabled()) {
            ui->spinBoxAmount->setEnabled(true);
            ui->labelPrice->setText(STR("���ۣ�"));
        }
    }

    data.assetData.type = AssetCode2Type::getInstance()->getAssetType(data.assetData.assetCode);
}
void myExchangeFormStock::on_codeLineEdit_editingFinished() {
    int count = stockCode->codeName.count();
    qDebug() << STR("����EditLine") << ui->codeLineEdit->text() << "(" << count << ")";
    if (OTHER != grpMarket->checkedId()) {
        if (stockCode->getIsInitialed()) {
            ui->nameLineEdit->setText(stockCode->findNameFromCode(data.assetData.assetCode));
        }
    }

    // check and show remain stock amount
    if (isShowRemainStock && accountNode) {
        myAssetNode *assetNode = nullptr;
        int remainStock = 0;
        for (int i = 0; i < accountNode->children.count(); i++) {
            assetNode = accountNode->children.at(i);
            if (data.assetData.assetCode == assetNode->nodeData.value<myAssetHold>().assetData.accountCode) {
                remainStock = assetNode->nodeData.value<myAssetHold>().assetData.amount;
                QFont font;
                font.setItalic(true);
                ui->spinBoxAmount->setFont(font);
                ui->spinBoxAmount->setValue(remainStock);
                break;
            }
        }
    }
}
void myExchangeFormStock::on_nameLineEdit_editingFinished() {
    QString str = ui->nameLineEdit->text();
    int count = stockCode->codeName.count();
    qDebug() << STR("����EditLine") << str << "(" << count << ")";

    QMap<QString,QString>::const_iterator it = stockCode->codeName.begin();
    for (; it != stockCode->codeName.end(); ++it) {
        if (it.value() == str) {
            data.assetData.assetCode = it.key();
            QRegExp rx("([a-zA-Z]*)[.][0-9]*");
            int pos = data.assetData.assetCode.indexOf(rx);
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

            ui->codeLineEdit->setText(data.assetData.assetCode);
            qDebug() << data.assetData.assetCode << it.value();
            return;
        }
    }

    ui->radioOther->setChecked(true);
    updateMarketInfo();
}

void myExchangeFormStock::on_spinBoxAmount_valueChanged(int value) {
    data.assetData.amount = buySellFlag*qAbs(value);
    updateBuySell();
}
void myExchangeFormStock::on_spinBoxPrice_valueChanged(double value) {
    data.assetData.price = value;
    updateBuySell();
}

void myExchangeFormStock::on_radioBuy_clicked() {
    qDebug() << "#radioBuy_clicked#";
    updateBuySell();
    data.exchangeType = STR("֤ȯ����");
    setExchangeWindowType(data.exchangeType);
}
void myExchangeFormStock::on_radioSell_clicked() {
    qDebug() << "#radioSell_clicked#";
    updateBuySell();
    data.exchangeType = STR("֤ȯ����");
    setExchangeWindowType(data.exchangeType);
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
    // 1. data.accountMoney & data.account2 update
    accountNode = rootNode->getAccountNode(nodeIdx);
    const myAssetAccount accountData = accountNode->nodeData.value<myAssetAccount>();
    data.accountMoney = accountData.accountData.code;
    data.assetData.accountCode = accountData.accountData.code;
    // 2. totalMoney
    totalMoney = getTotalMoney(nodeIdx);
    if (isModifyExchange) {
        totalMoney -= usedMoneyBeforeModify;
    }
    ui->moneySpinBoxTotal->setValue(totalMoney);
    qDebug() << "#moneyAccount_currentIndexChanged# isModifyExchange:" << isModifyExchange
             << " totalMoney:" << totalMoney;
    // 3. aommisionRate
    commisionRate = accountNode->nodeData.value<myAssetAccount>().accountData.note.toDouble();
    ui->feeRateSpinBox->setValue(commisionRate*1000);
    qDebug() << STR("Ӷ��") << commisionRate;
    // 4. fee & money
    updateExchangeFee();
    ui->moneySpinBox->setValue(calcMoney());
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
    ui->moneySpinBox->setValue(calcMoney());
}

void myExchangeFormStock::on_nameLineEdit_textChanged(const QString &name) {
    data.assetData.assetName = name;
}
