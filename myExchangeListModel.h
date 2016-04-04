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

    bool doExchange(const myExchangeData &data, bool isDelete = false);
    bool initial();

    myExchangeData getDataFromRow(int row);

    /*
     * changeIdx:
     * 1  changes -> origin account1
     * 2  changes -> origin account2
     * 4  changes -> origin/target account1
     * 8  changes -> origin/target account2
     * 16 changes -> origin/target account1/account2
     */
    enum exchangeType {
        NO_DO_EXCHANGE = 0x0,
        ORIG_ACCOUNT_1 = 0x01,
        ORIG_ACCOUNT_2 = 0x02,
        TARG_ACCOUNT_1 = 0x04,
        TARG_ACCOUNT_2 = 0x08,
        CASH_ACCOUNT_2 = 0x10,
        OTHER_EXCHANGE = 0x20
    };

    void coordinatorModifyExchange(myExchangeData &originData, myExchangeData &targetData, int &changeIdx);

private:
    QStringList list;
    myExchangeData *data;

    QString updateStrFromExchangeData(const myExchangeData &exchangeData);
};

#endif // MYEXCHANGELISTMODEL_H
