#ifndef MYASSETHISTORY_H
#define MYASSETHISTORY_H
#include "myGlobal.h"

#include "myAssetNode.h"
#include "myExchangeListNode.h"
#include "myStockHistoryData.h"

#include <QDateTime>
#include <QList>

class myAssetHistory
{
public:
    myAssetHistory();
    myAssetHistory(const myAccountAssetRootNode &root, const myExchangeListNode &exchangeListNode)
        : currentAssetTime(QDateTime::currentDateTime()), historyRoot(root), exchangeListNode(exchangeListNode) {}
    ~myAssetHistory();

    const myAccountAssetRootNode &getHistoryNode(const QDateTime &time);
    bool doExchangeNode(const myExchangeData &exchangeData);

    static void getHistoryDataList(const QList<QString> &currentStockHolding);

private:
    QDateTime currentAssetTime;
    QList<QString> currentStockHolding;
    void calcCurrentStockHolding();
    myStockHistoryData *stockHistoryData;

    myAccountAssetRootNode historyRoot;
    myExchangeListNode exchangeListNode;

    bool doChange(const myAssetData &assetData);
};

#endif // MYASSETHISTORY_H
