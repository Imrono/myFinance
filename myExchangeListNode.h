#ifndef MYEXCHANGELISTNODE_H
#define MYEXCHANGELISTNODE_H
#include "myGlobal.h"

#include "myDatabaseDatatype.h"

#include <QList>

class myExchangeListNode
{
public:
    myExchangeListNode();
    myExchangeListNode(const myExchangeListNode &otherNode);
    myExchangeListNode &operator =(const myExchangeListNode &otherNode);
    ~myExchangeListNode() {}

    myExchangeListNode &deepCopy(const myExchangeListNode &otherNode);

    bool initial();
    void callback();
    bool doExchange(myExchangeData &exchangeData, bool isDelete);

    const myExchangeData &getDataFromRow(int row) const {
        return exchangeList.at(row);
    }
    const myExchangeData *getDataPtrFromRow(int row) const {
        return &exchangeList[row];
    }
    const QList<myExchangeData>& getExchangeList() const {
        return exchangeList;
    }

    int getRowCount() const {
        return exchangeList.count();
    }

private:
    QList<myExchangeData> exchangeList;

    void updateList(const myExchangeData &exchangeData);
};

#endif // MYEXCHANGELISTNODE_H
