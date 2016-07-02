#include "myExchangeFormFund.h"
#include "ui_myExchangeFormFund.h"

myExchangeFormFund::myExchangeFormFund(const myAccountAssetRootNode *rootNode, QString tabName, QWidget *parent, bool isModifyExchange) :
    myExchangeFormTabBase(rootNode, tabName, myExchangeUI::TAB_STOCK, parent, isModifyExchange),
    totalAssetValue(0.0f), remainAssetValue(0.0f), currentAccount(nullptr), currentAsset(nullptr),
    ui(new Ui::myExchangeFormFund)
{
    ui->setupUi(this);
    ui->netValueSpinBox->setValue(1.0f);

    grpOperation = new QButtonGroup(this);
    grpOperation->addButton(ui->radioSubscribing);
    grpOperation->addButton(ui->radioRedeming);
    grpOperation->setExclusive(true);         //设为互斥
    grpOperation->setId(ui->radioSubscribing, SUBSCRIBING);
    grpOperation->setId(ui->radioRedeming,    REDEMING);
    ui->radioSubscribing->click();

    // 控件ComboBox -> ACCOUNT
    exchangeIdx2AccountIdx.clear();
    int localCount = 0;
    for (int i = 0; i < rootNode->getAccountCount(); i++) {
        const myAccountNode *account = rootNode->getAccountNode(i);
        const myAccountNodeData &accountInfo = GET_CONST_ACCOUNT_NODE_DATA(account);
        if (!accountInfo.accountData.name.contains(STR("银行")))
            continue;

        QIcon   icon = QIcon(QString(":/icon/finance/resource/icon/finance/%1").arg(accountInfo.logo));
        QString code = "**** **** " + accountInfo.accountData.code.right(4);

        exchangeIdx2AccountIdx.insert(localCount, i);
        ui->moneyAccount->addItem(icon, code);
        localCount++;
    }

    // 控件ComboBox -> ASSET TYPE
    ui->typeBox->addItem(STR("R1（谨慎型）"));
    ui->typeBox->addItem(STR("R2（稳健型）"));
    ui->typeBox->addItem(STR("R3（平衡型）"));
    ui->typeBox->addItem(STR("R4（进取型）"));
    ui->typeBox->addItem(STR("R5（激进型）"));
    ui->typeBox->addItem(STR("货币基金"));
    ui->typeBox->setCurrentIndex(0);
}

myExchangeFormFund::~myExchangeFormFund()
{
    delete ui;
}

void myExchangeFormFund::exchangeWindowFeeChanged(double fee) {
    qDebug() << "$$myExchangeFormFund::exchangeWindowFeeChanged " << fee << "$$";
    myExchangeFormTabBase::exchangeWindowFeeChanged(fee);
    ui->moneySpinBox->setValue(getMoneyUsed());
}

void myExchangeFormFund::recordExchangeData(myExchangeData &tmpData) {
    myExchangeFormTabBase::recordExchangeData(tmpData);

    if (currentAccount) {
        tmpData.accountMoney      = GET_CONST_ACCOUNT_NODE_DATA(currentAccount).accountData.code;
    }
    tmpData.money                 = ui->moneySpinBox->value();

    tmpData.assetData.accountCode = data.accountMoney;
    tmpData.assetData.assetCode   = ui->codeLineEdit->text();
    tmpData.assetData.assetName   = ui->nameLineEdit->text();
    tmpData.assetData.amount      = data.assetData.amount;
    tmpData.assetData.price       = ui->usedSpinBox->value();
    tmpData.assetData.type        = data.assetData.type;
}

void myExchangeFormFund::setUI(const myExchangeData &exchangeData) {
    int index = ui->moneyAccount->findText(exchangeData.assetData.accountCode);
    ui->moneyAccount->setCurrentIndex(exchangeIdx2AccountIdx.find(index).value());
    ui->nameLineEdit->setText(exchangeData.assetData.assetCode);
    ui->codeLineEdit->setText(exchangeData.assetData.assetName);
    ui->usedSpinBox->setValue(exchangeData.assetData.price);
    if (STR("认购") == exchangeData.exchangeType) {
        ui->radioSubscribing->click();
    } else if (STR("赎回") == exchangeData.exchangeType) {
        ui->radioRedeming->click();
    } else {}

    myExchangeFormTabBase::setUI(exchangeData);
}

void myExchangeFormFund::checkAndSetDisable(const myExchangeData &exchangeData) {
    setUI(exchangeData);
    if (exchangeData.accountMoney == exchangeData.assetData.accountCode && exchangeData.accountMoney != "")
        ui->moneyAccount->setDisabled(true);
    if (exchangeData.assetData.assetCode != "")
        ui->codeLineEdit->setDisabled(true);
    if (exchangeData.assetData.assetName != "")
        ui->nameLineEdit->setDisabled(true);
}

void myExchangeFormFund::updateBuySell() {
    ui->remainSpinBox->setValue(totalAssetValue + getAssetValue());
    ui->moneySpinBox->setValue(getMoneyUsed());
}
void myExchangeFormFund::on_radioSubscribing_clicked() {
    ui->checkBoxSoldAll->setDisabled(true);
    buySellFlag = 1.0f;
    data.exchangeType = STR("认购");
    setExchangeWindowType(data.exchangeType);
    data.assetData.amount = buySellFlag*qAbs(data.assetData.amount);

    updateBuySell();
}
void myExchangeFormFund::on_radioRedeming_clicked() {
    ui->checkBoxSoldAll->setEnabled(true);
    buySellFlag = -1.0f;
    data.exchangeType = STR("赎回");
    setExchangeWindowType(data.exchangeType);
    data.assetData.amount = buySellFlag*qAbs(data.assetData.amount);

    updateBuySell();
}

void myExchangeFormFund::on_netValueSpinBox_valueChanged(double value) {
    data.assetData.price = value;
    double amount = 0.0f;
    if (currentAsset) {
        const myAssetNodeData &assetHold = GET_CONST_ASSET_NODE_DATA(currentAsset);
        if (data.assetData.assetCode == assetHold.assetData.assetCode) {
            amount = assetHold.assetData.amount;
        }
    }

    ui->keepsSpinBox->setValue(data.assetData.price * amount);
}
void myExchangeFormFund::on_keepsSpinBox_valueChanged(double value) {
    totalAssetValue = value;

    ui->remainSpinBox->setValue(totalAssetValue + getAssetValue());
}
void myExchangeFormFund::on_usedSpinBox_valueChanged(double value) {
    data.assetData.amount = value/data.assetData.price * buySellFlag;
    qDebug() << STR("#myExchangeFormFund::on_usedSpinBox_valueChanged data.assetData.amount=%1").arg(data.assetData.amount);

    updateBuySell();
}
void myExchangeFormFund::on_remainSpinBox_valueChanged(double value) {
    remainAssetValue = value;
}

void myExchangeFormFund::on_moneySpinBoxTotal_valueChanged(double value) {
    totalMoney = value;

    ui->moneySpinBoxRemain->setValue(totalMoney + data.money);
    qDebug() << "#myExchangeFormFund::moneySpinBoxTotal_valueChanged# remainMoney:" << remainMoney
             << " data.money:" << data.money << " totalMoney:" << totalMoney;
}
void myExchangeFormFund::on_moneySpinBox_valueChanged(double value) {
    data.money = value;

    ui->moneySpinBoxRemain->setValue(totalMoney + data.money);
    qDebug() << "#myExchangeFormFund::moneySpinBox_valueChanged# remainMoney:" << remainMoney
             << " data.money:" << data.money << " totalMoney:" << totalMoney;
}
void myExchangeFormFund::on_moneySpinBoxRemain_valueChanged(double value) {
    remainMoney = value;
}

void myExchangeFormFund::on_codeLineEdit_textEdited(const QString &str) {
    data.assetData.assetCode = str;

    currentAsset = nullptr;
    if (currentAccount) {
        for (int i = 0; i < currentAccount->children.count(); i++) {
            const myAssetNode *asset = static_cast<const myAssetNode *>(currentAccount->children.at(i));
            const myAssetNodeData &assetHold = GET_CONST_ASSET_NODE_DATA(asset);
            if (data.assetData.assetCode == assetHold.assetData.assetCode) {
                currentAsset = asset;
                break;
            }
        }
    }
    ui->nameLineEdit->setText(getCurrentAssetName());

    totalAssetValue = getAssetCodeValue(data.assetData.assetCode);
    ui->keepsSpinBox->setValue(totalAssetValue);
}
void myExchangeFormFund::on_nameLineEdit_textChanged(const QString &str) {
    data.assetData.assetName = str;
    qDebug() << STR("#nameLineEdit_textChanged -> %1 #").arg(data.assetData.assetName);
}
float myExchangeFormFund::getAssetCodeValue(const QString &code) {
    float total = 0.0f;
    if (currentAccount) {
        for (int i = 0; i < currentAccount->children.count(); i++) {
            const myAssetNode *asset = static_cast<const myAssetNode *>(currentAccount->children.at(i));
            const myAssetNodeData &assetHold = GET_CONST_ASSET_NODE_DATA(asset);
            if (code == assetHold.assetData.assetCode) {
                total = assetHold.assetData.price * assetHold.assetData.amount;
                break;
            }
        }
    }
    return total;
}

void myExchangeFormFund::on_typeBox_currentIndexChanged(int index) {
    Q_UNUSED(index);
    data.assetData.type = ui->typeBox->currentText();
    qDebug() << STR("#typeBox_currentIndexChanged -> %1#").arg(data.assetData.type);
}

void myExchangeFormFund::on_moneyAccount_currentIndexChanged(int index) {
    int nodeIdx = exchangeIdx2AccountIdx.find(index).value();
    // data.accountMoney & data.account2 update
    currentAccount = rootNode->getAccountNode(nodeIdx);
    if (currentAccount) {
        const myAccountNodeData &accountInfo = GET_CONST_ACCOUNT_NODE_DATA(currentAccount);
        data.accountMoney = accountInfo.accountData.code;
        data.assetData.accountCode = accountInfo.accountData.code;

        totalMoney = getAssetCodeValue(MY_CASH);
        ui->moneySpinBoxTotal->setValue(totalMoney);

        totalAssetValue = getAssetCodeValue(data.assetData.assetCode);
        ui->keepsSpinBox->setValue(totalAssetValue);
    }
}

void myExchangeFormFund::on_checkBoxSoldAll_clicked() {
    if (ui->checkBoxSoldAll->isChecked()) {
        ui->usedSpinBox->setReadOnly(true);
        ui->usedSpinBox->setValue(totalAssetValue);
    } else {
        ui->usedSpinBox->setReadOnly(false);
        ui->usedSpinBox->setValue(0.0f);
    }
}
