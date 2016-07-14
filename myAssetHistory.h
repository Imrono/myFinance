#ifndef MYASSETHISTORY_H
#define MYASSETHISTORY_H
#include "myGlobal.h"
#include "myAssetNode.h"
#include "myExchangeListNode.h"

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

private:
    QDateTime currentAssetTime;
    myAccountAssetRootNode historyRoot;
    myExchangeListNode exchangeListNode;
};

#endif // MYASSETHISTORY_H
