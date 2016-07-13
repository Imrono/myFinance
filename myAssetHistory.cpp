#include "myAssetHistory.h"

#include <QDateTime>

myAssetHistory::myAssetHistory()
    : currentAssetTime(QDateTime::currentDateTime())
{
    historyRoot.initial();
}

myAssetHistory::~myAssetHistory() {
    historyRoot.callback();
}

const myAccountAssetRootNode &myAssetHistory::getHistoryNode(const QDateTime &time) {
    // historyRoot calculate
    qint64 refMsec = time.toMSecsSinceEpoch();
    if (refMsec >= currentAssetTime.toMSecsSinceEpoch())
        return historyRoot;

    int exchangeCount = exchangeList.getRowCount();
    qint64 lastMsec = exchangeList.getDataFromRow(exchangeCount-1).time.toMSecsSinceEpoch();
    for (int i = exchangeCount-1; i >= 0; i--) {
        qint64 exchangeMsec = exchangeList.getDataFromRow(i).time.toMSecsSinceEpoch();
        if (lastMsec != exchangeMsec)
            lastMsec = exchangeMsec;

        if (exchangeMsec > currentAssetTime.toMSecsSinceEpoch()) {
            continue;
        } else if (exchangeMsec > refMsec) {
            // do sth. with historyRoot using exchangeList.at(i)
        }
    }

    currentAssetTime = time;
    return historyRoot;
}
