#ifndef MYEXCHANGELISTMODEL_H
#define MYEXCHANGELISTMODEL_H

#include <QStringListModel>
#include "myDatabaseDatatype.h"
#include "myFinanceDatabase.h"

class myExchangeListModel : public QStringListModel
{
    Q_OBJECT
public:
    myExchangeListModel();
    ~myExchangeListModel();

    void test() {
        //beginResetModel();
        list.append("aaaa");
        //endResetModel();
        setStringList(list);
    }

    bool doExchange(const myExchangeData data);
    bool initial();

    myExchangeData getDataFromRow(int row);

    enum coordinatorType {
        NO_CHANGE = 0,
        ACCOUNT1_CHANGE = 0x01,
        ACCOUNT2_CHANGE = 0x02,
        MONEY_CHANGE = 0x04,
        ASSET_CHANGE = 0x08,
        OTHER_CHANGE = 0x10
    };

    void coordinatorModifyExchange(myExchangeData &originData, myExchangeData &targetData, int &type);

private:
    QStringList list;
    myExchangeData *data;
};

#endif // MYEXCHANGELISTMODEL_H
