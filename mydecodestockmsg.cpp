#include "myStockHistoryData.h"

#include <QDate>

QString historyDailyDataProcessThread::stockCode2YahooStyle(const QString &stockCode) {
    QString tmpStockCode = stockCode;
    QString preStr = tmpStockCode.left(3);
    if ("sh." == preStr) {
        tmpStockCode.remove(0, 3);
        tmpStockCode.append(STR(".ss"));
    } else if ("sz." == preStr) {
        tmpStockCode.remove(0, 3);
        tmpStockCode.append(STR(".sz"));
    } else {}
    return tmpStockCode;
}

QString historyDailyDataProcessThread::stockUrlYahoo(const QString &stockCode) {
    QString urlYahoo = STR("http://table.finance.yahoo.com/table.csv?s=%1")
            .arg(stockCode2YahooStyle(stockCode));
    return urlYahoo;
}

bool historyDailyDataProcessThread::stockDecodeYahoo(QByteArray &lineData, myStockDailyData &dailyData) {
    if (lineData == "Date,Open,High,Low,Close,Volume,Adj Close\n") {
        return false;
    }
    lineData.remove(lineData.lastIndexOf('\n'), 1);
    QList<QByteArray> strList = lineData.split(',');

    if (7 == strList.count()) {
        dailyData.datetime = QDateTime::fromString(strList.at(0), "yyyy-MM-dd");
        dailyData.open     = strList.at(1).toFloat();
        dailyData.high     = strList.at(2).toFloat();
        dailyData.low      = strList.at(3).toFloat();
        dailyData.close    = strList.at(4).toFloat();
        dailyData.volume   = strList.at(5).toInt();
        dailyData.adjClose = strList.at(6).toFloat();
        return true;
    }
    return false;
}

QString historyDailyDataProcessThread::stockCode2NetEaseStyle(const QString &stockCode) {
    QString tmpStockCode = stockCode;
    QString preStr = tmpStockCode.left(3);
    if ("sh." == preStr) {
        tmpStockCode.remove(0, 3);
        tmpStockCode.insert(0, "0");
    } else if ("sz." == preStr) {
        tmpStockCode.remove(0, 3);
        tmpStockCode.insert(0, "1");
    } else {}
    return tmpStockCode;
}

QString historyDailyDataProcessThread::stockUrlNetEase(const QString &stockCode) {
    QString urlNetEase = STR("http://quotes.money.163.com/service/chddata.html?"
                         "code=%1&start=00000000&end=%2"
                         "&fields=TCLOSE;HIGH;LOW;TOPEN;LCLOSE;CHG;PCHG;TURNOVER;VOTURNOVER;VATURNOVER;TCAP;MCAP")
            .arg(stockCode2NetEaseStyle(stockCode))
            .arg(QDate::currentDate().toString("yyyyMMdd"));
    return urlNetEase;
}

bool historyDailyDataProcessThread::stockDecodeNetEase(QByteArray &lineData, myStockDailyData &dailyData) {
    QString title = STR("日期,股票代码,名称,收盘价,最高价,最低价,开盘价,前收盘,涨跌额,涨跌幅,换手率,成交量,成交金额,总市值,流通市值");
    QString s;
    s.prepend(lineData);
    s.contains(title);
    if (lineData.contains(title.toUtf8())) {
        return false;
    }
    lineData.remove(lineData.lastIndexOf('\n'), 1);
    QList<QByteArray> strList = lineData.split(',');

    if (7 == strList.count()) {
        dailyData.datetime = QDateTime::fromString(strList.at(0), "yyyy-MM-dd");
        dailyData.open     = strList.at(6).toFloat();
        dailyData.high     = strList.at(4).toFloat();
        dailyData.low      = strList.at(5).toFloat();
        dailyData.close    = strList.at(3).toFloat();
        dailyData.volume   = strList.at(11).toInt();
        dailyData.adjClose = strList.at(7).toFloat();
        return true;
    }
    return false;
}
