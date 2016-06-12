#include "myFinanceMainWindow.h"
#include <QApplication>
#include <QtCore/QTextCodec>
#include <QPixmap>
#include <QSplashScreen>
#include <QElapsedTimer>

#include "myFinanceDatabase.h"
#include "myStockCodeName.h"
#include "AssetCode2Type.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QPixmap pixmap(":/SplashScreen/resource/SplashScreen/gold10years.jpg");
    QSplashScreen screen(pixmap);
    screen.show();

    QElapsedTimer timer;
    timer.start();

    ///////////////////////////////////////////////////////////////////////
    /// 初始化数据
    myFinanceDatabase::connectDB();
    myFinanceDatabase::initialDB();
    myStockCodeName::initial();
    //AssetCode2Type::initial();
    ///////////////////////////////////////////////////////////////////////
    myFinanceMainWindow w;
    while(timer.elapsed() < (600))  // 初始化结束后，再等600ms
        app.processEvents();
    w.show();

    screen.finish(&w);

    return app.exec();
}
