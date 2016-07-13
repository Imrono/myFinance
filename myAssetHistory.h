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
    myAssetHistory(const myAccountAssetRootNode &root, const myExchangeListNode &exchangeList)
        : currentAssetTime(QDateTime::currentDateTime()), historyRoot(root), exchangeList(exchangeList) {}
    ~myAssetHistory();

    const myAccountAssetRootNode &getHistoryNode(const QDateTime &time);
private:
    QDateTime currentAssetTime;
    myAccountAssetRootNode historyRoot;
    myExchangeListNode exchangeList;
};

#endif // MYASSETHISTORY_H
