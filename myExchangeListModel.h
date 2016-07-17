#ifndef MYEXCHANGELISTMODEL_H
#define MYEXCHANGELISTMODEL_H
#include "myGlobal.h"

#include "myDatabaseDatatype.h"
#include "myExchangeListNode.h"

#include <QStringListModel>
#include <QList>

class myExchangeListModel : public QStringListModel
{
    Q_OBJECT
public:
    myExchangeListModel(QObject *parent = 0);
    ~myExchangeListModel();

    bool doExchange(myExchangeData &data, bool isDelete = false, bool isSyncWithDb = true);
    void doReflash();

    const myExchangeData &getDataFromRow(int row) const {
        return exchangeNode.getDataFromRow(row);
    }

    /*
     * changeIdx:
     * 1  changes -> origin accountMoney
     * 2  changes -> origin account2
     * 4  changes -> origin/target accountMoney
     * 8  changes -> origin/target account2
     * 16 changes -> origin/target accountMoney/account2
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

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QString *stringFromIndex(const QModelIndex &index) const;

private:
    myExchangeListNode exchangeNode;

    QString updateStrFromExchangeData(const myExchangeData &exchangeData) const;
};

#endif // MYEXCHANGELISTMODEL_H
