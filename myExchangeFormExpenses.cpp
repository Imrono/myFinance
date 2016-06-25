#include "myExchangeFormExpenses.h"
#include "ui_myExchangeFormExpenses.h"

myExchangeFormExpenses::myExchangeFormExpenses(const myAccountAssetRootNode *rootNode, QString tabName, QWidget *parent) :
    myExchangeFormTabBase(rootNode, tabName, myExchangeUI::TAB_EXPES, parent),
    ui(new Ui::myExchangeFormExpenses)
{
    ui->setupUi(this);

    ui->lineEditExpendCode->setText(STR("类别"));
    ui->lineEditExpendName->setText(STR("名称"));

    spendIdx2AccountIdx.clear();
    int localCount = 0;
    for (int i = 0; i < rootNode->getAccountCount(); i++) {
        myAssetNode *accountNode = rootNode->getAccountNode(i);
        if (accountNode->nodeData.value<myAssetAccount>().accountData.type.contains(STR("券商"))) {
                continue;
        }
        for (int j = 0; j < accountNode->children.count(); j++) {
            myAssetNode *holdNode = accountNode->children.at(j);
            QString assetCode = holdNode->nodeData.value<myAssetHold>().assetData.assetCode;
            if (assetCode.contains("cash")) {
                const myAssetAccount &accountData = accountNode->nodeData.value<myAssetAccount>();
                QIcon   icon = QIcon(QString(":/icon/finance/resource/icon/finance/%1").arg(accountData.logo));
                QString code;
                if (accountData.accountData.name.contains(STR("银行"))) {
                    code = "**** **** " + accountData.accountData.code.right(4);
                } else {
                    code = accountData.accountData.code;
                }
                spendIdx2AccountIdx.insert(localCount, i);
                ui->moneyAccountExpend->addItem(icon, code);
                localCount ++;
                break;
            }
        }
    }
}

myExchangeFormExpenses::~myExchangeFormExpenses()
{
    delete ui;
}

void myExchangeFormExpenses::recordExchangeData(myExchangeData &tmpData) {
    myExchangeFormTabBase::recordExchangeData(tmpData);

    tmpData.accountMoney = ui->moneyAccountExpend->itemText(ui->moneyAccountExpend->currentIndex());
    tmpData.money    = -ui->spinBoxExpend->value();

    tmpData.assetData.accountCode = OTHER_ACCOUNT;
    tmpData.assetData.assetCode   = ui->lineEditExpendCode->text();
    tmpData.assetData.assetCode   = ui->lineEditExpendName->text();
    tmpData.assetData.amount      = 1;
    tmpData.assetData.price       = ui->spinBoxExpend->value();
}
void myExchangeFormExpenses::setUI(const myExchangeData &exchangeData) {

    myExchangeFormTabBase::setUI(exchangeData);
}
