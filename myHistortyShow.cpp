#include "myHistortyShow.h"
#include "ui_myHistortyShow.h"

#include <qwt_date_scale_draw.h>
#include <qwt_date_scale_engine.h>

myHistortyShow::myHistortyShow(const QMap<QDateTime, double> &historyValue, QWidget *parent) :
    QDialog(parent), historyValue(historyValue),
    ui(new Ui::myHistortyShow)
{
    ui->setupUi(this);

    plot = new QwtPlot(ui->historyPlot);
    plot->setTitle("Plot Demo");
    plot->setCanvasBackground( Qt::white);
    plot->setAxisScale(QwtPlot::yLeft, 1.0, 10.0);
    //plot->insertLegend(new QwtLegend());

    QwtPlotGrid *grid = new QwtPlotGrid();
    grid->attach(plot);

    QwtPlotCurve *curve = new QwtPlotCurve();
    curve->setTitle("Some Points");
    curve->setPen(Qt::blue, 4),
    curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);

    QwtSymbol *symbol = new QwtSymbol(QwtSymbol::Ellipse,
        QBrush(Qt::yellow), QPen( Qt::red, 2), QSize(8, 8));
    curve->setSymbol(symbol);

    QwtDateScaleDraw *timeScale = new QwtDateScaleDraw();
    QwtDateScaleEngine *scaleEngine = new QwtDateScaleEngine();

    timeScale->setDateFormat(QwtDate::Second, "hh:mm:ss\nyy-MM-dd");
    timeScale->setDateFormat(QwtDate::Millisecond, "hh:mm:ss\nyy-MM-dd");

    plot->setAxisScaleDraw(QwtPlot::xBottom, timeScale);
    plot->setAxisScaleEngine(QwtPlot::xBottom, scaleEngine);
    uint currentTime = QDateTime::currentDateTime().toTime_t();
    const uint msecDay = 1000*3600*24;
    plot->setAxisScale(QwtPlot::xBottom, currentTime, currentTime+msecDay*5, 1000*3600*24);

    plot->setAxisLabelRotation(QwtPlot::xBottom, -50.0);
    plot->setAxisLabelAlignment(QwtPlot::xBottom, Qt::AlignLeft | Qt::AlignBottom);

    QPolygonF points;
    points << QPointF(0.0, 4.4) << QPointF(1.0, 3.0)
        << QPointF(2.0, 4.5) << QPointF(3.0, 6.8)
        << QPointF(4.0, 7.9) << QPointF(5.0, 7.1);
    curve->setSamples(points);

    curve->attach(plot);

    plot->resize(600, 400);
    plot->show();
}

myHistortyShow::~myHistortyShow()
{
    delete ui;
}
