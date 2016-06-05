﻿#include "myFinanceExchangeWindow.h"
#include "ui_myFinanceExchangeWindow.h"

#include <QtCore/QMap>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QRadioButton>
#include <QMessageBox>
#include "myFinanceMainWindow.h"

#include <QtDebug>

myFinanceExchangeWindow::myFinanceExchangeWindow(QWidget *parent, unsigned winType) :
    QDialog(parent), ui(new Ui::myFinanceExchangeWindow), winType(winType),
    isRollback(false), stockCode(myStockCodeName::getInstance()),
    _currentTab(nullptr), dataSource(-1)
{
    ui->setupUi(this);

    ui->timeDateTimeEdit->setDateTime(QDateTime::currentDateTime());
    ui->typeLineEdit->setReadOnly(true);

    rootNode = &static_cast<myFinanceMainWindow *>(parent)->getAssetModel()->getRootNode();

    _myTabs[TAB_STOCK] = winType&TYPE_STOCK ? new myExchangeFormStock   (rootNode, STR("股票交易"), this) : nullptr;
    _myTabs[TAB_TRANS] = winType&TYPE_TRANS ? new myExchangeFormTransfer(rootNode, STR("转帐"),     this) : nullptr;
    _myTabs[TAB_INCOM] = winType&TYPE_INCOM ? new myExchangeFormIncome  (rootNode, STR("收入"),     this) : nullptr;
    _myTabs[TAB_EXPES] = winType&TYPE_EXPES ? new myExchangeFormExpenses(rootNode, STR("支出"),     this) : nullptr;

    for (int i = 0; i < MAX_TAB_COUNT; i++) {
        unsigned tmpTab = 0x01 << i;
        if (winType & tmpTab) {
            if (-1 == dataSource) { //默认值是第一个
                dataSource = i;
                _currentTab = _myTabs[dataSource];
                ui->tabWidget->setCurrentIndex(dataSource);
            }
            ui->tabWidget->addTab(_myTabs[i], _myTabs[i]->getTabText());
        }
    }

    // ROLLBACK CHECKBOX
    ui->checkBoxRollback->setChecked(isRollback);
    ui->checkBoxRollback->hide();
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
        QMessageBox::warning(this, "checkDataConsistence", "checkDataConsistence Failed", QMessageBox::Discard, QMessageBox::Discard);
    }
    // 2. 更新新tab
    dataSource = index;
    _currentTab = _myTabs[dataSource];
    qDebug() << "## CURRENT TAB: " << dataSource << " ##";
    // 3. 更新新tab中的公共数据
    _currentTab->recoverTypeAndFee();
}

void myFinanceExchangeWindow::showRollback() {
    ui->checkBoxRollback->setVisible(true);
}
void myFinanceExchangeWindow::setUI(const myExchangeData &exchangeData, bool rollbackShow) {
    const myExchangeData data = exchangeData;
    ui->timeDateTimeEdit->setDateTime(data.time);
    ui->typeLineEdit->setText(data.type);
    if (qAbs(data.fee) > MONEY_EPS) {
        ui->exchangeFeeSpinBox->setValue(data.fee);
    } else {
        double fee = -(static_cast<double>(data.amount)*data.price + data.money);
        ui->exchangeFeeSpinBox->setValue(fee);
    }
    if (rollbackShow) {
        showRollback();
    }

    if (data.type.contains(STR("证券"))) {
        ui->tabWidget->setCurrentIndex(TAB_STOCK);
    }  else if (data.type.contains(STR("转帐"))) {
        ui->tabWidget->setCurrentIndex(TAB_TRANS);
    } else if (data.type.contains(STR("收入"))) {
        ui->tabWidget->setCurrentIndex(TAB_INCOM);
    } else if (data.type.contains(STR("支出"))) {
        ui->tabWidget->setCurrentIndex(TAB_EXPES);
    } else {}

    _currentTab->setUI(data);
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
    tmpData.type = ui->typeLineEdit->text();
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
