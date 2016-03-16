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
    void doExchange(const exchangeData data);

private:
    QStringList list;
};

#endif // MYEXCHANGELISTMODEL_H
