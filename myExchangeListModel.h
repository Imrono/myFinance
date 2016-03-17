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
    bool doExchange(const exchangeData data);
    bool initial();

private:
    QStringList list;
};

#endif // MYEXCHANGELISTMODEL_H
