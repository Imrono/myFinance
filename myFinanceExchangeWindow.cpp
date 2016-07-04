#include "myFinanceExchangeWindow.h"
#include "ui_myFinanceExchangeWindow.h"

#include <QtCore/QMap>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QRadioButton>
#include <QMessageBox>
#include "myFinanceMainWindow.h"

#include <QtDebug>

myFinanceExchangeWindow::myFinanceExchangeWindow(QWidget *parent, const myExchangeUI &exchangeUI, bool isPartial, bool isModifyExchange) :
    QDialog(parent), ui(new Ui::myFinanceExchangeWindow), parent(static_cast<myFinanceMainWindow *>(parent)),
    isRollback(false), stockCode(myStockCodeName::getInstance()),
    _currentTab(nullptr), dataSource(0)
{
    qDebug() << "################## INITIAL EXCHANGE WINDOW ##################";
    rootNode = &static_cast<myFinanceMainWindow *>(parent)->getAssetModel()->getRootNode();

    ui->setupUi(this);

    // ROLLBACK CHECKBOX HIDE AS DEFAULT
    ui->checkBoxRollback->setChecked(isRollback);
    ui->checkBoxRollback->hide();

    // COMMON UI UPDATE
    ui->timeDateTimeEdit->setDateTime(QDateTime::currentDateTime());
    ui->typeLineEdit->setReadOnly(true);

    initialTabs(exchangeUI, isPartial, isModifyExchange);
}

void myFinanceExchangeWindow::initialTabs(const myExchangeUI &exchangeUI, bool isPartial, bool isModifyExchange) {
    /// 1. 默认4个tab，默认的setUI
    /// 2. 单个tab，用exchangeUI中数据setUI
    if (myExchangeUI::MAX_TAB_COUNT == exchangeUI.getNumOfTabs()) {
        _myTabs.append(new myExchangeFormStock   (rootNode, STR("证券交易"), this));
        _myTabs.append(new myExchangeFormMoneyUp (rootNode, STR("理财")    , this));
        _myTabs.append(new myExchangeFormFund    (rootNode, STR("基金")    , this));
        _myTabs.append(new myExchangeFormTransfer(rootNode, STR("转帐")    , this));
        //_myTabs.append(new myExchangeFormIncome  (rootNode, STR("收入")    , this));
        //_myTabs.append(new myExchangeFormExpenses(rootNode, STR("支出")    , this));
        _currentTab = _myTabs[dataSource];
        _currentTab->recoverTypeAndFee();
    } else if (1 == exchangeUI.getNumOfTabs()) {
        if (exchangeUI.getTabType() == myExchangeUI::TAB_STOCK)
            _myTabs.append(new myExchangeFormStock   (rootNode, STR("证券交易"), this, isModifyExchange));
        if (exchangeUI.getTabType() == myExchangeUI::TAB_TRANS)
            _myTabs.append(new myExchangeFormTransfer(rootNode, STR("转帐")    , this, isModifyExchange));
        //if (exchangeUI.getTabType() == myExchangeUI::TAB_INCOM)
        //    _myTabs.append(new myExchangeFormIncome  (rootNode, STR("收入")    , this));
        //if (exchangeUI.getTabType() == myExchangeUI::TAB_EXPES)
        //    _myTabs.append(new myExchangeFormExpenses(rootNode, STR("支出")    , this));
        if (exchangeUI.getTabType() == myExchangeUI::TAB_MONUP)
            _myTabs.append(new myExchangeFormMoneyUp(rootNode, STR("理财")    , this));
        if (exchangeUI.getTabType() == myExchangeUI::TAB_FUNDS)
            _myTabs.append(new myExchangeFormFund   (rootNode, STR("基金")    , this));
        if (_myTabs.count() == 0) {
            qDebug() << "ERROR: NO TAB IS ADDED";
            return;
        }
        _currentTab = _myTabs[dataSource];
        if (isPartial) {
            setUI(exchangeUI.getExchangeData(), exchangeUI.getIsShowRollback());
            _currentTab->checkAndSetDisable(exchangeUI.getExchangeData());
        }
        qDebug() << "## SETUP UI MODEL FINISH ##";
    } else {  }

    for (int i = 0; i < _myTabs.count(); i++) {
        _myTabs[i]->setDateTime(dateTime);
        if (STR("证券交易") == _myTabs[i]->getTabText()) {
            QIcon icon(STR(":/icon/exchangeTab/resource/icon/exchangeTab/mySecurity.png"));
            ui->tabWidget->addTab(_myTabs[i], icon, _myTabs[i]->getTabText());
        } else if (STR("理财") == _myTabs[i]->getTabText()) {
            QIcon icon(STR(":/icon/exchangeTab/resource/icon/exchangeTab/myMoneyUp.png"));
            ui->tabWidget->addTab(_myTabs[i], icon, _myTabs[i]->getTabText());
        } else if (STR("基金") == _myTabs[i]->getTabText()) {
            QIcon icon(STR(":/icon/exchangeTab/resource/icon/exchangeTab/myFund.png"));
            ui->tabWidget->addTab(_myTabs[i], icon, _myTabs[i]->getTabText());
        } else if (STR("转帐") == _myTabs[i]->getTabText()) {
            QIcon icon(STR(":/icon/exchangeTab/resource/icon/exchangeTab/myTransfer.png"));
            ui->tabWidget->addTab(_myTabs[i], icon, _myTabs[i]->getTabText());
        } else {
            ui->tabWidget->addTab(_myTabs[i], _myTabs[i]->getTabText());
        }
        qDebug() << _myTabs[i]->getTabText() << "ADDED to Exchange Window";
    }
    ui->tabWidget->setCurrentIndex(dataSource);
}

myFinanceExchangeWindow::~myFinanceExchangeWindow() {
    delete ui;
}

void myFinanceExchangeWindow::on_exchangeFeeSpinBox_valueChanged(double value) {
    qDebug() << "#exchangeFeeSpinBox_valueChanged# fee:" << value;
    _currentTab->exchangeWindowFeeChanged(value);
}

void myFinanceExchangeWindow::on_tabWidget_currentChanged(int index)
{
    // 1. 检查之前tab中数据的一致性
    if (!checkDataConsistence()) {
        QMessageBox::warning(this, "checkDataConsistence", _currentTab->getTabText()+": checkDataConsistence Failed", QMessageBox::Discard, QMessageBox::Discard);
    }
    // 2. 更新新tab
    dataSource = index;
    _currentTab = _myTabs[dataSource];
    qDebug() << "## CURRENT TAB: " << dataSource << " with tabName " << _currentTab->getTabText() << " ##";
    // 3. 更新新tab中的公共数据
    _currentTab->recoverTypeAndFee();
    _currentTab->setDateTime(dateTime);
}

void myFinanceExchangeWindow::showRollback() {
    ui->checkBoxRollback->setVisible(true);
}
void myFinanceExchangeWindow::setUI(const myExchangeData &exchangeData, bool rollbackShow) {
    ui->timeDateTimeEdit->setDateTime(exchangeData.time);
    ui->typeLineEdit->setText(exchangeData.exchangeType);
    if (qAbs(exchangeData.fee) > MONEY_EPS) {
        ui->exchangeFeeSpinBox->setValue(exchangeData.fee);
    } else {
        double fee = -(static_cast<double>(exchangeData.assetData.amount)*exchangeData.assetData.price + exchangeData.money);
        ui->exchangeFeeSpinBox->setValue(fee);
    }
    if (rollbackShow) {
        showRollback();
    }

    _currentTab->setUI(exchangeData);
}

/////////////////////////////////////////////////////////////////////////////
void myFinanceExchangeWindow::on_checkBoxRollback_clicked() {
    isRollback = (Qt::Checked == ui->checkBoxRollback->checkState());
    qDebug() << "checkBoxRollback_clicked: isRollback:" << isRollback;
}

void myFinanceExchangeWindow::setExchangeWindowUiType(const QString type) {
    ui->typeLineEdit->setText(type);
}
void myFinanceExchangeWindow::setExchangeWindowUiFee(double fee) {
    ui->exchangeFeeSpinBox->setValue(fee);
}
void myFinanceExchangeWindow::getCommonExchangeData(myExchangeData &tmpData) {
    tmpData.time = ui->timeDateTimeEdit->dateTime();
    tmpData.exchangeType = ui->typeLineEdit->text();
    tmpData.fee  = ui->exchangeFeeSpinBox->value();
}

bool myFinanceExchangeWindow::checkDataConsistence() {
    myExchangeData tmpData;
    _currentTab->recordExchangeData(tmpData);
    if (tmpData == _currentTab->getExchangeData()) {
        qDebug() << "tab " << _currentTab->getTabText() << "Data Consistence OK";
        return true;
    } else {
        qDebug() << "tab " << _currentTab->getTabText() << "Data Consistence NOK";
        return false;
    }
}

void myFinanceExchangeWindow::on_timeDateTimeEdit_dateTimeChanged(const QDateTime &dateTime) {
    this->dateTime = dateTime;
    qDebug() << "DATE TIME " << dateTime.toString();
}
