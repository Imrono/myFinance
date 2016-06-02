#include "myExchangeFormExpenses.h"
#include "ui_myExchangeFormExpenses.h"

myExchangeFormExpenses::myExchangeFormExpenses(const myRootAccountAsset *rootNode, QString tabName, QWidget *parent) :
    myExchangeFormTabBase(rootNode, tabName, parent),
    ui(new Ui::myExchangeFormExpenses)
{
    ui->setupUi(this);

    ui->lineEditExpendCode->setText(STR("类别"));
    ui->lineEditExpendName->setText(STR("名称"));

    spendIdx2AccountIdx.clear();
    int localCount = 0;
    for (int i = 0; i < rootNode->getAccountCount(); i++) {
        myAssetNode *accountNode = rootNode->getAccountNode(i);
        if (accountNode->nodeData.value<myAssetAccount>().type.contains(STR("券商"))) {
                continue;
        }
        for (int j = 0; j < accountNode->children.count(); j++) {
            myAssetNode *holdNode = accountNode->children.at(j);
            QString assetCode = holdNode->nodeData.value<myAssetHold>().assetCode;
            if (assetCode.contains("cash")) {
                const myAssetAccount &accountData = accountNode->nodeData.value<myAssetAccount>();
                QIcon   icon = QIcon(QString(":/icon/finance/resource/icon/finance/%1").arg(accountData.logo));
                QString code;
                if (accountData.name.contains(STR("银行"))) {
                    code = "**** **** " + accountData.code.right(4);
                } else {
                    code = accountData.code;
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

    tmpData.account1 = ui->moneyAccountExpend->itemText(ui->moneyAccountExpend->currentIndex());
    tmpData.money    = -ui->spinBoxExpend->value();

    tmpData.account2 = OTHER_ACCOUNT;
    tmpData.code     = ui->lineEditExpendCode->text();
    tmpData.name     = ui->lineEditExpendName->text();
    tmpData.amount   = 1;
    tmpData.price    = ui->spinBoxExpend->value();
}
void myExchangeFormExpenses::setUI(const myExchangeData &exchangeData) {

    myExchangeFormTabBase::setUI(exchangeData);
}
