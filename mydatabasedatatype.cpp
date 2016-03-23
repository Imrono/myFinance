#include "myDatabaseDatatype.h"

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
