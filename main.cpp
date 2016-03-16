#include "myFinanceMainWindow.h"
#include <QApplication>
#include <QtCore/QTextCodec>

#include "myFinanceDatabase.h"
#include "myStockCodeName.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    myFinanceDatabase::connectDB();

    myStockCodeName stockCode;

    myFinanceMainWindow w(&stockCode);
    w.show();


    return a.exec();
}
