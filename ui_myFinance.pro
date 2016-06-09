#-------------------------------------------------
#
# Project created by QtCreator 2016-01-31T21:49:57
#
#-------------------------------------------------

QT       += core gui sql network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app


SOURCES += main.cpp\
        myFinanceMainWindow.cpp \
    myAssetModel.cpp \
    myAssetNode.cpp \
    myFinanceExchangeWindow.cpp \
    myStockCodeName.cpp \
    myStockPrice.cpp \
    myExchangeListModel.cpp \
    myFinanceDatabase.cpp \
    myInsertModifyAccount.cpp \
    myInsertModifyAsset.cpp \
    assetChangeDelegate.cpp \
    myDatabaseDatatype.cpp \
    myModifyExchange.cpp \
    myGlobal.cpp \
    myMoneyIncomeExpenses.cpp \
    myExchangeFormIncome.cpp \
    myExchangeFormStock.cpp \
    myExchangeFormExpenses.cpp \
    myExchangeFormTransfer.cpp \
    myExchangeFormTabBase.cpp \
    myFinanceTreeVeiwContextMenu.cpp

HEADERS  += myFinanceMainWindow.h \
    myAssetModel.h \
    myAssetNode.h \
    myDatabaseDatatype.h \
    myFinanceExchangeWindow.h \
    myStockCodeName.h \
    myStockPrice.h \
    myExchangeListModel.h \
    myFinanceDatabase.h \
    myInsertModifyAccount.h \
    myInsertModifyAsset.h \
    assetChangeDelegate.h \
    myModifyExchange.h \
    myGlobal.h \
    myMoneyIncomeExpenses.h \
    myExchangeFormIncome.h \
    myExchangeFormStock.h \
    myExchangeFormExpenses.h \
    myExchangeFormTransfer.h \
    myExchangeFormTabBase.h \
    myFinanceTreeVeiwContextMenu.h

FORMS    += myFinanceMainWindow.ui \
    myFinanceExchangeWindow.ui \
    myInsertModifyAccount.ui \
    myInsertModifyAsset.ui \
    myModifyExchange.ui \
    myExchangeFormIncome.ui \
    myExchangeFormStock.ui \
    myExchangeFormExpenses.ui \
    myExchangeFormTransfer.ui

CONFIG(debug, debug|release) {
    CONFIG += console
    TARGET = myFinance_d
} else {
    CONFIG +=
    TARGET = myFinance
}
#CONFIG += console

DISTFILES +=

RESOURCES += resfinance.qrc

RC_ICONS = resource/icon/myLogo.ico
