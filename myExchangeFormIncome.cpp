#include "myExchangeFormIncome.h"
#include "ui_myExchangeFormIncome.h"

myExchangeFormIncome::myExchangeFormIncome(const myAccountAssetRootNode *rootNode, QString tabName, QWidget *parent) :
    myExchangeFormTabBase(rootNode, tabName, myExchangeUI::TAB_INCOM, parent),
    ui(new Ui::myExchangeFormIncome)
{
    ui->setupUi(this);

    grpIncomeType = new QButtonGroup(this);
    grpIncomeType->addButton(ui->radioSalary);
    grpIncomeType->addButton(ui->radioOtherIncome);
    grpIncomeType->setExclusive(true);              //设为互斥
    grpIncomeType->setId(ui->radioSalary, 0);       //radioSalary的Id设为0
    grpIncomeType->setId(ui->radioOtherIncome, 1);  //radioOtherIncome的Id设为1
    ui->radioSalary->setChecked(true);
    ui->lineEditIncomeType->setDisabled(true);

    incomeIdx2AccountIdx.clear();
    int localCount = 0;
    for (int i = 0; i < rootNode->getAccountCount(); i++) {
        myAssetNode *accountNode = rootNode->getAccountNode(i);
        if (accountNode->nodeData.value<myAccountNodeData>().accountData.type.contains(STR("券商"))) {
                continue;
        }
        for (int j = 0; j < accountNode->children.count(); j++) {
            myAssetNode *holdNode = accountNode->children.at(j);
            QString assetCode = holdNode->nodeData.value<myAssetNodeData>().assetData.assetCode;
            if (assetCode.contains("cash")) {
                const myAccountNodeData &accountData = accountNode->nodeData.value<myAccountNodeData>();
                QIcon   icon = QIcon(QString(":/icon/finance/resource/icon/finance/%1").arg(accountData.logo));
                QString code;
                if (accountData.accountData.name.contains(STR("银行"))) {
                    code = "**** **** " + accountData.accountData.code.right(4);
                } else {
                    code = accountData.accountData.code;
                }
                incomeIdx2AccountIdx.insert(localCount, i);
                ui->moneyAccountIncome->addItem(icon, code);
                localCount ++;
                break;
            }
        }
    }
}

myExchangeFormIncome::~myExchangeFormIncome()
{
    delete ui;
}

void myExchangeFormIncome::recordExchangeData(myExchangeData &tmpData) {
    myExchangeFormTabBase::recordExchangeData(tmpData);

    tmpData.assetData.accountCode = ui->moneyAccountIncome->itemText(ui->moneyAccountIncome->currentIndex());
    tmpData.assetData.assetCode   = MY_CASH;
    if (grpIncomeType->checkedId() == 0) {
        tmpData.assetData.assetName = STR("工资收入");
    } else if (grpIncomeType->checkedId() == 1) {
        tmpData.assetData.assetName = ui->lineEditIncomeType->text();
    } else {}
    tmpData.assetData.amount = 1;
    tmpData.assetData.price  = ui->spinBoxIncome->value();

    tmpData.accountMoney = OTHER_ACCOUNT;
    tmpData.money    = -data.assetData.price;
}
void myExchangeFormIncome::setUI(const myExchangeData &exchangeData) {

    myExchangeFormTabBase::setUI(exchangeData);
}

void myExchangeFormIncome::on_radioSalary_clicked() {
    qDebug() << "radioSalary clicked";
    updateIncomeType();
}
void myExchangeFormIncome::on_radioOtherIncome_clicked() {
    qDebug() << "radioOtherIncome clicked";
    updateIncomeType();
}
void myExchangeFormIncome::on_lineEditIncomeType_textChanged(const QString &str) {
    data.assetData.assetName = str;
}
void myExchangeFormIncome::updateIncomeType() {
    if (grpIncomeType->checkedId() == 0) {
        data.assetData.assetName = STR("工资收入");
        ui->lineEditIncomeType->setDisabled(true);
    } else if (grpIncomeType->checkedId() == 1) {
        ui->lineEditIncomeType->setEnabled(true);
        data.assetData.assetName = ui->lineEditIncomeType->text();
    } else {}
}
