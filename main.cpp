#include "myFinanceMainWindow.h"
#include <QApplication>
#include <QtCore/QTextCodec>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    myFinanceMainWindow w;
    w.show();


    return a.exec();
}
