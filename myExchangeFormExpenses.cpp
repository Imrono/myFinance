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
        const myAccountNode *accountNode = rootNode->getAccountNode(i);
        if (GET_CONST_ACCOUNT_NODE_DATA(accountNode).accountData.type.contains(STR("券商"))) {
                continue;
        }
        for (int j = 0; j < accountNode->children.count(); j++) {
            const myAssetNode *asset = static_cast<const myAssetNode *>(accountNode->children.at(j));
            QString assetCode = GET_CONST_ASSET_NODE_DATA(asset).assetData.assetCode;
            if (assetCode.contains("cash")) {
                const myAccountNodeData &accountInfo = GET_CONST_ACCOUNT_NODE_DATA(accountNode);
                QIcon   icon = QIcon(QString(":/icon/finance/resource/icon/finance/%1").arg(accountInfo.logo));
                QString code;
                if (accountInfo.accountData.name.contains(STR("银行"))) {
                    code = "**** **** " + accountInfo.accountData.code.right(4);
                } else {
                    code = accountInfo.accountData.code;
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
