#ifndef MYFINANCEDATABASE_H
#define MYFINANCEDATABASE_H

#include <QtSql/QSqlDatabase>

class myFinanceDatabase
{
public:
    myFinanceDatabase();

    static bool connectDB();
    static QSqlDatabase db;
    static bool isConnected;
};

#endif // MYFINANCEDATABASE_H
