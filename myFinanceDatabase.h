#ifndef MYFINANCEDATABASE_H
#define MYFINANCEDATABASE_H
#include "myGlobal.h"

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>

class myFinanceDatabase
{
public:
    myFinanceDatabase();

    static bool connectDB();
    static bool initialDB();
    static QSqlDatabase db;
    static bool isConnected;
    static QString dbType;

    static int getQueryRows(const QString &execWord);
};

#endif // MYFINANCEDATABASE_H
