#-------------------------------------------------
#
# Project created by QtCreator 2016-01-31T21:49:57
#
#-------------------------------------------------

QT       += core gui sql network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ui_myFinance
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
    myModifyExchange.cpp

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
    myModifyExchange.h

FORMS    += myFinanceMainWindow.ui \
    myFinanceExchangeWindow.ui \
    myInsertModifyAccount.ui \
    myInsertModifyAsset.ui \
    myModifyExchange.ui
CONFIG += console

DISTFILES +=

RESOURCES += resfinance.qrc

RC_ICONS = resource/icon/myLogo.ico
