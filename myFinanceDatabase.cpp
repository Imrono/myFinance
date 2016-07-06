#include "myFinanceDatabase.h"
#include "myGlobal.h"
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlDriver>

#include <QDebug>

QSqlDatabase myFinanceDatabase::db;
bool myFinanceDatabase::isConnected = false;
QString myFinanceDatabase::dbType = "QSQLITE";

myFinanceDatabase::myFinanceDatabase()
{

}

bool myFinanceDatabase::connectDB() {
    dbType = "QSQLITE";
//  dbType = "QMYSQL";
    if (db.isOpen()) {
        db.close();
    } else {
        db = QSqlDatabase::addDatabase(dbType);
        if ("QSQLITE" == dbType) {
            db.setDatabaseName("myFinance.db");
        } else if ("QMYSQL" == dbType) {
            db.setDatabaseName("myfinance");
            db.setHostName("127.0.0.1");
            db.setUserName("root");
            db.setPassword("");
        }
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

bool myFinanceDatabase::initialDB() {
    if (!isConnected) {
        return false;
    } else {
        QSqlQuery query;
        QString execWord = STR("CREATE TABLE IF NOT EXISTS 资产 ("
                               "代号 varchar(16), "
                               "名称 varchar(16), "
                               "资产帐户代号 varchar(32), "
                               "数量 decimal(14,3) DEFAULT 0, "
                               "单位成本 decimal(14,3) DEFAULT 0, "
                               "类别 varchar(16), "
                               "pos int(4) DEFAULT -1, "
                               "显示类别 int(4) DEFAULT -1,"
                               "primary key (代号, 资产帐户代号));");
        MY_DEBUG_SQL(execWord);
        if(!query.exec(execWord)) {
            MY_DEBUG_ERROR(query.lastError().text());
            return false;
        }
        execWord = STR("CREATE TABLE IF NOT EXISTS 资产变化 ("
                       "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                       "时间 datetime DEFAULT CURRENT_TIMESTAMP, "
                       "变化类别 varchar(16), "
                       "资产帐户代号1 varchar(32), "
                       "变化资金 decimal(14,3), "
                       "资产帐户代号2 varchar(32), "
                       "代号 varchar(16), "
                       "名称 varchar(16), "
                       "单价 decimal(14,3) DEFAULT 0, "
                       "数量 decimal(14,3) DEFAULT 0);");
        MY_DEBUG_SQL(execWord);
        if(!query.exec(execWord)) {
            MY_DEBUG_ERROR(query.lastError().text());
            return false;
        }
        execWord = STR("CREATE TABLE IF NOT EXISTS 资产帐户 ("
                       "代号 varchar(32) PRIMARY KEY, "
                       "名称 varchar(16), "
                       "类别 varchar(16), "
                       "备注 varchar(32), "
                       "pos int(4) DEFAULT -1);");
        MY_DEBUG_SQL(execWord);
        if(!query.exec(execWord)) {
            MY_DEBUG_ERROR(query.lastError().text());
            return false;
        }
        execWord = STR("CREATE TABLE IF NOT EXISTS 现金收支 ("
                       "代号 varchar(32) PRIMARY KEY, "
                       "名称 varchar(16), "
                       "类别 varchar(16), "
                       "备注 varchar(32), "
                       "pos int(4) DEFAULT -1);");
        MY_DEBUG_SQL(execWord);
        if(!query.exec(execWord)) {
            MY_DEBUG_ERROR(query.lastError().text());
            return false;
        }
        execWord = STR("CREATE TABLE IF NOT EXISTS 收支种类ui ("
                       "名称 varchar(16) PRIMARY KEY, "
                       "类别 varchar(16), "
                       "备注 varchar(32), "
                       "pos int(4) DEFAULT -1,"
                       "收支 varchar(16));");
        MY_DEBUG_SQL(execWord);
        if(!query.exec(execWord)) {
            MY_DEBUG_ERROR(query.lastError().text());
            return false;
        }
    }
    MY_A_TRACE("### DB initial SUCCESS!! ###");
    return true;
}

// numRows < 0 -> ERROR
int myFinanceDatabase::getQueryRows(const QString &execWord) {
    int numRows = -1;
    QSqlQuery query;
    MY_DEBUG_SQL(execWord);
    if(query.exec(execWord)) {
        query.next();
        numRows = query.value(0).toInt();
        query.finish();
        return numRows;
    } else {
        MY_DEBUG_ERROR(query.lastError().text());
        return -1;
    }
    return numRows;
}
