#include "myFinanceMainWindow.h"
#include <QApplication>
#include <QtCore/QTextCodec>

#include "myStockCodeName.h"
int main(int argc, char *argv[])
{
    myStockCodeName tmp;
    tmp.getStockCode();

    QApplication a(argc, argv);
    myFinanceMainWindow w;
    w.show();


    return a.exec();
}
