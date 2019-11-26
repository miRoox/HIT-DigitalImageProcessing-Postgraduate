#include "histogramview.h"
#include "algorithms.h"
#include <numeric>
#include <QtCharts/QLineSeries>
#include <QtCharts/QAreaSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChartView>
#include <QVector>
#include <QPointF>
#include <QHBoxLayout>

using namespace QtCharts;

QVector<QPointF> linePointsFromHistogram(const HistogramView::Histogram& hist)
{
    QVector<QPointF> points(0x100*2);
    for (int i=0;i<0x100;++i)
    {
        points.append(QPointF{1.0*i,hist[i]});
        points.append(QPointF{1.0*(i+1),hist[i]});
    }
    return points;
}

HistogramView::HistogramView(QWidget *parent)
    : QWidget(parent), data(0x100,0), chart(new QChart)
{
    auto lseries = new QLineSeries();
    lseries->append(linePointsFromHistogram(data).toList());

    auto series = new QAreaSeries(lseries);
    QPen pen(Qt::gray);
    pen.setWidth(1);
    series->setPen(pen);

    QLinearGradient gradient(QPointF(0, 0), QPointF(1, 0));
    gradient.setColorAt(0.0, Qt::black);
    gradient.setColorAt(1.0, Qt::white);
    gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
    series->setBrush(gradient);

    chart->addSeries(lseries);
    chart->createDefaultAxes();
    chart->axes(Qt::Horizontal).first()->setRange(0, 256);
    chart->axes(Qt::Vertical).first()->setMin(0);

    auto view = new QChartView(chart);
    view->setRenderHint(QPainter::Antialiasing);

    auto layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->addWidget(view);
    setLayout(layout);

    connect(this,&HistogramView::histogramChanged,
            this,&HistogramView::updateChart);
}

HistogramView::Histogram HistogramView::histogram() const
{
    return data;
}

QString HistogramView::title() const
{
    return chart->title();
}

void HistogramView::calculateImageHistogram(const QImage &image)
{
    setHistogram(::histogram(image));
}

void HistogramView::setHistogram(const HistogramView::Histogram &hist)
{
    Q_ASSERT_X(hist.size()==0x100,__func__,"Unsuitable size.");
    Q_ASSERT_X(qFuzzyCompare(std::accumulate(hist.cbegin(),hist.cend(),0.0),1.0),__func__,"Sum of frequencies is not one.");
    if (data==hist)
        return;
    data = hist;
    emit histogramChanged(data);
}

void HistogramView::setTitle(const QString &title)
{
    chart->setTitle(title);
}

void HistogramView::updateChart(const HistogramView::Histogram &hist)
{
    auto series = qobject_cast<QAreaSeries*>(chart->series().first());
    Q_CHECK_PTR(series);
    auto line = series->upperSeries();
    line->replace(linePointsFromHistogram(hist));
}
