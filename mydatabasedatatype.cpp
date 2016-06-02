#include "myDatabaseDatatype.h"

myExchangeData::myExchangeData() {
    id       = -1;
    time     = QDateTime();
    type     = "";
    account1 = "";
    money    = 0.0f;
    account2 = "";
    code     = "";
    name     = "";
    price    = 0.0f;
    amount   = 0;
    fee      = 0.0f;
}
myExchangeData myExchangeData::operator -() {
    myExchangeData tmp;
    tmp = *this;
    tmp.money = -tmp.money;
    tmp.fee = -tmp.fee;
    if (code == MY_CASH) {
        tmp.price = -tmp.price;
    } else {
        tmp.amount = -tmp.amount;
    }
    return tmp;
}
myExchangeData &myExchangeData::operator =(const myExchangeData &data) {
    id       = data.id;
    time     = data.time;
    type     = data.type;
    account1 = data.account1;
    money    = data.money;
    account2 = data.account2;
    code     = data.code;
    name     = data.name;
    price    = data.price;
    amount   = data.amount;
    fee      = data.fee;
    return *this;
}

bool &myExchangeData::operator ==(const myExchangeData &data) {
    bool ans = true;
    ans = (time     == data.time)          && ans;
    ans = (type     == data.type)          && ans;
    ans = (account1 == data.account1)      && ans;
    ans = ((money-data.money) < MONEY_EPS) && ans;
    ans = (account2 == data.account2)      && ans;
    ans = (code     == data.code)          && ans;
    ans = (name     == data.name)          && ans;
    ans = ((price-data.price) < MONEY_EPS) && ans;
    ans = (amount   == data.amount)        && ans;
    ans = ((fee-data.fee) < MONEY_EPS)     && ans;
    return ans;
}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
bool myAccountData::isSameAccountData(const myAccountData &data1, const myAccountData &data2) {
    return (   data1.Code == data2.Code
            && data1.Name == data2.Name
            && data1.Type == data2.Type
            && data1.Note == data2.Note);
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

myAccountData::myAccountData(myAssetAccount data) {
    Code = data.code;
    Name = data.name;
    Type = data.type;
    Note = data.note;
}

myAssetData::myAssetData() {

}

myAssetData::myAssetData(myAssetHold data) {
    accountCode = data.accountCode;
    assetCode   = data.assetCode;
    assetName   = data.name;
    amount      = data.amount;
    price       = data.price;
    type        = data.type;
}
