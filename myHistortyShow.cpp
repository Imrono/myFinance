#include "myHistortyShow.h"
#include "ui_myHistortyShow.h"
#include "myAssetHistory.h"

#include <qwt_date_scale_draw.h>
#include <qwt_date_scale_engine.h>

historyPlot::historyPlot(const QMap<QDateTime, double> &historyValue, QWidget *parent)
    : historyValue(historyValue), QwtPlot(parent){
    setTitle("Plot Demo");
    setCanvasBackground( Qt::white);
    //setAxisScale(QwtPlot::yLeft, 1.0, 3000.0);
    //insertLegend(new QwtLegend());

    QwtPlotGrid *grid = new QwtPlotGrid();
    grid->attach(this);

    QwtPlotCurve *curve = new QwtPlotCurve();
    curve->setTitle("Some Points");
    curve->setPen(Qt::blue, 4),
    curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);

    QwtSymbol *symbol = new QwtSymbol(QwtSymbol::Ellipse, QBrush(Qt::yellow),
                                      QPen( Qt::red, 2), QSize(8, 8));
    curve->setSymbol(symbol);

    QwtDateScaleDraw *timeScale = new QwtDateScaleDraw();
    timeScale->setDateFormat(QwtDate::Day, "yy-MM-dd");
    timeScale->setDateFormat(QwtDate::Second, "hh:mm:ss\nyy-MM-dd");
    timeScale->setDateFormat(QwtDate::Millisecond, "hh:mm:ss\nyy-MM-dd");
    setAxisScaleDraw(QwtPlot::xBottom, timeScale);

    QwtDateScaleEngine *scaleEngine = new QwtDateScaleEngine();
    setAxisScaleEngine(QwtPlot::xBottom, scaleEngine);

    double beginTime_d = QwtDate::toDouble(historyValue.begin().key());
    double lastTime_d  = QwtDate::toDouble((--historyValue.end()).key());
    setAxisScale(QwtPlot::xBottom, beginTime_d, lastTime_d, MSEC_DAY*7);

    setAxisLabelRotation(QwtPlot::xBottom, -50.0);
    setAxisLabelAlignment(QwtPlot::xBottom, Qt::AlignLeft | Qt::AlignBottom);

    QPolygonF points;
    qDebug() << "historyValueCount:" << historyValue.count();
    for (QMap<QDateTime, double>::const_iterator ii = historyValue.begin(); ii != historyValue.end(); ++ii) {
        qDebug() << ii.key() << " " << ii.value();
        points << QPointF(QwtDate::toDouble(ii.key()), ii.value());
        //points << QPointF(QwtDate::toDouble(ii.key()), 1);
    }
    curve->setSamples(points);

    curve->attach(this);

    resize(600, 400);
}

historyPlot::~historyPlot() {

}

myHistortyShow::myHistortyShow(const QMap<QDateTime, double> &historyValue, QWidget *parent) :
    QDialog(parent), ui(new Ui::myHistortyShow)
{
    ui->setupUi(this);

    plot = new historyPlot(historyValue, ui->historyPlot);
    plot->show();
}

myHistortyShow::~myHistortyShow()
{
    delete ui;
}
