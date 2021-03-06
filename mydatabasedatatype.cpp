﻿#include "myDatabaseDatatype.h"
#include <QDebug>

int getShowCategory(const myAssetData &tmpAssetHold) {
    Q_UNUSED(tmpAssetHold);
    return -1;
}

myExchangeData::myExchangeData() : assetData() {
    id       = -1;
    time     = QDateTime();
    exchangeType     = "";
    accountMoney = "";
    money    = 0.0f;
    fee      = 0.0f;
}
myExchangeData myExchangeData::operator -() {
    myExchangeData tmp;
    tmp = *this;
    tmp.money = -tmp.money;
    tmp.fee = -tmp.fee;
    if (assetData.assetCode == MY_CASH || assetData.assetCode == MY_MONEY_FUND) {
        tmp.assetData.price = -tmp.assetData.price;
    } else {
        tmp.assetData.amount = -tmp.assetData.amount;
    }
    return tmp;
}
myExchangeData &myExchangeData::operator =(const myExchangeData &data) {
    id           = data.id;
    time         = data.time;
    exchangeType = data.exchangeType;
    accountMoney = data.accountMoney;
    money        = data.money;
    assetData    = data.assetData;
    fee          = data.fee;
    return *this;
}

bool myExchangeData::operator ==(const myExchangeData &data) {
    bool ans = true;
    ans = (time     == data.time)             && ans;
    ans = (exchangeType == data.exchangeType) && ans;
    ans = (accountMoney == data.accountMoney) && ans;
    ans = ((money - data.money) < MONEY_EPS)  && ans;
    ans = assetData == data.assetData         && ans;
    return ans;
}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
bool myAccountData::isSameAccountData(const myAccountData &data1, const myAccountData &data2) {
    return (   data1.code == data2.code
            && data1.name == data2.name
            && data1.type == data2.type
            && data1.note == data2.note);
}

bool myAssetData::isSameAssetData(const myAssetData &data1, const myAssetData &data2) {
    return (   data1.accountCode == data2.accountCode
            && data1.assetCode == data2.assetCode
            && data1.assetName == data2.assetName
            && data1.type == data2.type
            && data1.amount == data2.amount
            && (qAbs(data1.price - data2.price) < MONEY_EPS));
}

#include "myAssetNode.h"
myAccountData::myAccountData() {

}

myAccountData::myAccountData(myAccountNodeData data) {
    *this = data.accountData;
}

myAssetData::myAssetData() {
    accountCode = "";
    assetCode   = "";
    assetName   = "";
    price       = 0.0f;
    amount      = 0;
    type        = "";
}

myAssetData::myAssetData(myAssetNodeData data) {
    *this = data.assetData;
}

myAssetData &myAssetData::operator =(const myAssetData &data) {
    accountCode = data.accountCode;
    assetCode   = data.assetCode;
    assetName   = data.assetName;
    price       = data.price;
    amount      = data.amount;
    type        = data.type;
    return *this;
}
bool myAssetData::operator ==(const myAssetData &data) {
    bool ans = true;
    ans = (accountCode == data.accountCode) && ans;
    ans = (assetCode   == data.assetCode)   && ans;
    ans = (assetName   == data.assetName)   && ans;
    ans = ((price-data.price) < MONEY_EPS)  && ans;
    ans = (amount      == data.amount)      && ans;
    ans = (type        == data.type)        && ans;
    return ans;
}
myAssetData myAssetData::operator +(const myAssetData &data) {
    if (data.assetCode == this->assetCode) {
        if (MY_CASH == this->assetCode
         && 1 == data.amount
         && 1 == this->amount) {
            this->price += data.price;
        } else if (MY_CASH != this->assetCode) {
            this->amount += data.amount;
        }
    }
    return *this;
}

void myAssetData::reset() {
    accountCode = "";
    assetCode   = "";
    assetName   = "";
    price       = 0.0f;
    amount      = 0;
    type        = "";
}

void myAssetData::initMoneyAsset(const QString &accountCode, const float money) {
    this->accountCode = accountCode;
    this->assetCode   = MY_CASH;
    this->assetName   = "";
    this->price       = money;
    this->amount      = 1;
    this->type        = "";
}

myDividends::myDividends() {
    shareSplit   = 0.0f;
    shareBonus   = 0.0f;
    capitalBonus = 0.0f;
    base = 0;
    type = myDividends::UNSPECIFIED;
}
