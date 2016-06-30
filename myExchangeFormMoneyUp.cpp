#include "myExchangeFormMoneyUp.h"
#include "ui_myExchangeFormMoneyUp.h"
#include "AssetCode2Type.h"

myExchangeFormMoneyUp::myExchangeFormMoneyUp(const myAccountAssetRootNode *rootNode, QString tabName, QWidget *parent) :
    myExchangeFormTabBase(rootNode, tabName, myExchangeUI::TAB_FUNDS, parent),
    currentAccount(nullptr), currentAsset(nullptr), caller(-1),
    totalAssetValue(0.0f), remainAssetValue(0.0f),
    totalMoney(0.0f), remainMoney(0.0f), benefits(0.0f),
    ui(new Ui::myExchangeFormMoneyUp)
{
    ui->setupUi(this);

    data.assetData.amount = 1;

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

myExchangeFormMoneyUp::~myExchangeFormMoneyUp()
{
    delete ui;
}

void myExchangeFormMoneyUp::exchangeWindowFeeChanged(double fee) {
    qDebug() << "$$myExchangeFormMoneyUp::exchangeWindowFeeChanged " << fee << "$$";
    myExchangeFormTabBase::exchangeWindowFeeChanged(fee);
    data.money = data.assetData.price - data.fee;
    ui->moneySpinBox->setValue(data.money);
}

void myExchangeFormMoneyUp::recordExchangeData(myExchangeData &tmpData) {
    myExchangeFormTabBase::recordExchangeData(tmpData);

    if (currentAccount) {
        tmpData.accountMoney      = GET_CONST_ACCOUNT_NODE_DATA(currentAccount).accountData.code;
    }
    tmpData.money                 = ui->moneySpinBox->value();

    tmpData.assetData.accountCode = data.accountMoney;
    tmpData.assetData.assetCode   = ui->codeLineEdit->text();
    tmpData.assetData.assetName   = ui->nameLineEdit->text();
    tmpData.assetData.amount      = 1;
    tmpData.assetData.price       = ui->usedSpinBox->value();
    tmpData.assetData.type        = data.assetData.type;
}

void myExchangeFormMoneyUp::setUI(const myExchangeData &exchangeData) {
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

void myExchangeFormMoneyUp::checkAndSetDisable(const myExchangeData &exchangeData) {
    setUI(exchangeData);
    if (exchangeData.accountMoney == exchangeData.assetData.accountCode && exchangeData.accountMoney != "")
        ui->moneyAccount->setDisabled(true);
    if (exchangeData.assetData.assetCode != "")
        ui->codeLineEdit->setDisabled(true);
    if (exchangeData.assetData.assetName != "")
        ui->nameLineEdit->setDisabled(true);
}

void myExchangeFormMoneyUp::on_moneyAccount_currentIndexChanged(int index) {
    int nodeIdx = exchangeIdx2AccountIdx.find(index).value();
    // 1. data.accountMoney & data.account2 update
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

void myExchangeFormMoneyUp::on_radioSubscribing_clicked() {
    ui->checkBoxSoldAll->setDisabled(true);
    buySellFlag = 1.0f;
    data.exchangeType = STR("认购");
    setExchangeWindowType(data.exchangeType);
    updateBuySell(FROM_RADIO_SUBSCRIBING);
}

void myExchangeFormMoneyUp::on_radioRedeming_clicked() {
    ui->checkBoxSoldAll->setEnabled(true);
    buySellFlag = -1.0f;
    data.exchangeType = STR("赎回");
    setExchangeWindowType(data.exchangeType);
    updateBuySell(FROM_RADIO_REDEMING);
}

void myExchangeFormMoneyUp::on_keepsSpinBox_valueChanged(double value) {
    totalAssetValue = value;
    ui->remainSpinBox->setValue(totalAssetValue + buySellFlag*data.assetData.price);

    updateBuySell(FROM_KEEPS_ASSET);
}

void myExchangeFormMoneyUp::on_remainSpinBox_valueChanged(double value) {
    remainAssetValue = value;
    updateBuySell(FROM_REAMIN_ASSET);
}

void myExchangeFormMoneyUp::on_usedSpinBox_valueChanged(double value) {
    data.assetData.price = buySellFlag * value;
    data.money = -buySellFlag * value - data.fee;
    //ui->remainSpinBox->setValue(totalAssetValue + buySellFlag*data.assetData.price);
    updateBuySell(FROM_EXCHANGE_ASSET);
}

void myExchangeFormMoneyUp::on_codeLineEdit_textChanged(const QString &str) {
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
    data.assetData.assetName = getCurrentAssetName();
    ui->nameLineEdit->setText(data.assetData.assetName);

    totalAssetValue = getAssetCodeValue(data.assetData.assetCode);
    ui->keepsSpinBox->setValue(totalAssetValue);
}

float myExchangeFormMoneyUp::getAssetCodeValue(const QString &code) {
    float total = 0.0f;
    if (currentAccount) {
        for (int i = 0; i < currentAccount->children.count(); i++) {
            const myAssetNode *asset = static_cast<const myAssetNode *>(currentAccount->children.at(i));
            const myAssetNodeData &assetHold = GET_CONST_ASSET_NODE_DATA(asset);
            if (code == assetHold.assetData.assetCode) {
                total = assetHold.assetData.price;
                break;
            }
        }
    }
    return total;
}

void myExchangeFormMoneyUp::updateBuySell(int caller) {
    qDebug() << updateMoney2String(caller) << " triggered buy&sell";


}

void myExchangeFormMoneyUp::on_moneySpinBoxTotal_valueChanged(double value) {
    totalMoney = value;
    remainMoney = totalMoney + data.money;
    ui->moneySpinBoxRemain->setValue(remainMoney);
    qDebug() << "#moneySpinBox_valueChanged# remainMoney:" << remainMoney
             << " data.money:" << data.money << " totalMoney:" << totalMoney;
}

void myExchangeFormMoneyUp::on_typeBox_currentIndexChanged(int index) {
    Q_UNUSED(index);
    data.assetData.type = ui->typeBox->currentText();
    qDebug() << STR("#typeBox_currentIndexChanged -> ") << data.assetData.type << "#";
}

void myExchangeFormMoneyUp::on_nameLineEdit_textChanged(const QString &str) {
    data.assetData.assetName = str;
    qDebug() << STR("#nameLineEdit_textChanged -> ") << data.assetData.assetName << "#";
}

void myExchangeFormMoneyUp::on_doDividendButton_clicked() {
    qDebug() << STR("理财 计算分红 clicked()");
    // 1. 读取分红数据

    // 2. 写入数据库并更新MainWindow

    // 3. 更新当前界面

}

void myExchangeFormMoneyUp::on_checkBoxSoldAll_clicked() {
    if (ui->checkBoxSoldAll->isChecked()) {
        ui->usedSpinBox->setReadOnly(true);
        ui->usedSpinBox->setValue(totalAssetValue);
    } else {
        ui->usedSpinBox->setReadOnly(false);
        ui->usedSpinBox->setValue(0.0f);
        return;
    }
}
