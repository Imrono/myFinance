#include "myHistortyShow.h"
#include "ui_myHistortyShow.h"
#include "myAssetHistory.h"

#include <qwt_date_scale_draw.h>
#include <qwt_date_scale_engine.h>

#include <QElapsedTimer>

historyPlot::historyPlot(QWidget *parent) : QwtPlot(parent) {

}
historyPlot::~historyPlot() {

}

myHistortyShow::myHistortyShow(const QMap<QDateTime, double> &historyValue, QWidget *parent) :
    QDialog(parent), historyValue(historyValue),
    ui(new Ui::myHistortyShow)
{
    ui->setupUi(this);

    myAssetHistory::historyValue_s.acquire();

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

    timeScale->setDateFormat(QwtDate::Day, "yy-MM-dd");
    timeScale->setDateFormat(QwtDate::Second, "hh:mm:ss\nyy-MM-dd");
    timeScale->setDateFormat(QwtDate::Millisecond, "hh:mm:ss\nyy-MM-dd");

    plot->setAxisScaleDraw(QwtPlot::xBottom, timeScale);
    plot->setAxisScaleEngine(QwtPlot::xBottom, scaleEngine);

    qDebug() << historyValue.count();
    for (QMap<QDateTime, double>::const_iterator ii = historyValue.begin(); ii != historyValue.end(); ++ii) {
        qDebug() << QwtDate::toDouble(ii.key()) << " " << ii.value();
    }
    double beginTime_d = QwtDate::toDouble(historyValue.begin().key());
    double lastTime_d  = QwtDate::toDouble((--historyValue.end()).key());
    plot->setAxisScale(QwtPlot::xBottom, beginTime_d, lastTime_d, MSEC_DAY);

    plot->setAxisLabelRotation(QwtPlot::xBottom, -50.0);
    plot->setAxisLabelAlignment(QwtPlot::xBottom, Qt::AlignLeft | Qt::AlignBottom);

    QPolygonF points;
    for (QMap<QDateTime, double>::const_iterator ii = historyValue.begin(); ii != historyValue.end(); ++ii) {
        qDebug() << ii.key() << " " << ii.value();
        points << QPointF(QwtDate::toDouble(ii.key()), ii.value());
        //points << QPointF(QwtDate::toDouble(ii.key()), 1);
    }
    curve->setSamples(points);

    curve->attach(plot);

    plot->resize(600, 400);
    plot->show();

    myAssetHistory::historyValue_s.release();
}

myHistortyShow::~myHistortyShow()
{
    delete ui;
}
