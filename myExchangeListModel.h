#ifndef MYEXCHANGELISTMODEL_H
#define MYEXCHANGELISTMODEL_H
#include "myGlobal.h"

#include <QStringListModel>
#include "myDatabaseDatatype.h"
#include "myFinanceDatabase.h"

class myExchangeListModel : public QStringListModel
{
    Q_OBJECT
public:
    myExchangeListModel();
    ~myExchangeListModel();

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
        ROLLBACK_ACCOUNT_1 = 0x01,
        ROLLBACK_ACCOUNT_2 = 0x02,
        TARG_ACCOUNT_1 = 0x04,
        TARG_ACCOUNT_2 = 0x08,
        OTHER_EXCHANGE = 0x10
    };

    void coordinatorModifyExchange(const myExchangeData &originData, const myExchangeData &targetData, int &changeIdx);

    QVariant data(const QModelIndex &index, int role) const;
    QString *stringFromIndex(const QModelIndex &index) const;

private:
    QStringList list;
    QList<myExchangeData> strData;

    QString updateStrFromExchangeData(const myExchangeData &exchangeData);
};

#endif // MYEXCHANGELISTMODEL_H
