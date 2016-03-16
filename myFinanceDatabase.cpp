#include "myFinanceDatabase.h"
#include <QtSql/QSqlError>

#include <QDebug>

QSqlDatabase myFinanceDatabase::db;
bool myFinanceDatabase::isConnected = false;

myFinanceDatabase::myFinanceDatabase()
{

}

bool myFinanceDatabase::connectDB() {
    if (db.isOpen()) {
        db.close();
    } else {
        db = QSqlDatabase::addDatabase("QMYSQL");
        db.setDatabaseName("myfinance");
        db.setHostName("127.0.0.1");
        db.setUserName("root");
        db.setPassword("");
    }

    if (!db.open()) {
        qDebug() << "Connect to MySql error: " << db.lastError().text();
        myFinanceDatabase::isConnected = false;
        return false;
    } else {
        myFinanceDatabase::isConnected = true;
        return true;
    }
}
