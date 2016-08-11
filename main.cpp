#include "myGlobal.h"

#include <QApplication>
#include <QtCore/QTextCodec>
#include <QPixmap>
#include <QSplashScreen>
#include <QElapsedTimer>

#include "myFinanceMainWindow.h"
#include "myFinanceDatabase.h"
#include "myStockCodeName.h"
#include "AssetCode2Type.h"

#include "myStockHistoryData.h"
#include "myAssetHistory.h"

#include <QDebug>
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    qDebug() << MY_FT_RED("WELLCOM") << MY_FT_YELLOW("TO") << MY_FT_GREEN("myFinance") << MY_FT_BOLD_BLACK("PROGRAM");
    qWarning() << "MAIN THREAD ID: " << QThread::currentThreadId();
    //myStockHistoryData *stockHistoryData = myStockHistoryData::getInstance();
    //stockHistoryData->insertStockHistory("sz.000333");

    //myAssetHistory assetHistory;
    //assetHistory.prepareCalcAssetValue(QDateTime(QDate(2016, 8, 4)), QDateTime(QDate(2016, 7, 19)));

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
    //while(timer.elapsed() < (600))  // 初始化结束后，再等600ms
    //    app.processEvents();
    w.show();

    screen.finish(&w);

    return app.exec();
}
