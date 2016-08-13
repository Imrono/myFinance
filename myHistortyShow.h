#ifndef MYHISTORTYSHOW_H
#define MYHISTORTYSHOW_H

#include <QDialog>
#include <QMap>
#include <QDateTime>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_symbol.h>
#include <qwt_legend.h>
#include <qwt_scale_draw.h>

class TimeScaleDraw: public QwtScaleDraw        //自画坐标轴
{
public:
    TimeScaleDraw()
    {
    }
    virtual QwtText label( double v ) const  //重绘坐标轴 刻度值
    {
        QDateTime dt;
        dt.setTime_t((int)v); //获取时间
        return dt.toString("mm:ss");  //转变为字符串  分秒  在刻度为最长的位置显示值
    }
};

class historyPlot : public QwtPlot
{
public:
    historyPlot(const QMap<QDateTime, double> &historyValue, QWidget *parent = NULL);
    ~historyPlot();

private:
    const QMap<QDateTime, double> historyValue;
};

namespace Ui {
class myHistortyShow;
}

class myHistortyShow : public QDialog
{
    Q_OBJECT

public:
    explicit myHistortyShow(const QMap<QDateTime, double> &historyValue, QWidget *parent = 0);
    ~myHistortyShow();

private:
    Ui::myHistortyShow *ui;
    historyPlot *plot;
};

#endif // MYHISTORTYSHOW_H
