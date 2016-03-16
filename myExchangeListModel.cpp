#include "myExchangeListModel.h"
#include "myFinanceDatabase.h"
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlError>
#include <QtSql/QSqlRecord>

myExchangeListModel::myExchangeListModel() {
    if (!myFinanceDatabase::isConnected) {
        myFinanceDatabase::connectDB();
    }
    setStringList(list);
}
myExchangeListModel::~myExchangeListModel() {

}

void myExchangeListModel::doExchange(const exchangeData data) {
    QSqlQuery query;
    // 1 write database

    // 2 read database

    // 3 更新list，刷新
    setStringList(list);
}
