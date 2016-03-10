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
    myStockPrice.cpp

HEADERS  += myFinanceMainWindow.h \
    myAssetModel.h \
    myAssetNode.h \
    myDatabaseDatatype.h \
    myFinanceExchangeWindow.h \
    myStockCodeName.h \
    myStockPrice.h

FORMS    += myFinanceMainWindow.ui \
    myFinanceExchangeWindow.ui
CONFIG += console

DISTFILES +=

RESOURCES +=
