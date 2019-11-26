#ifndef HISTOGRAMVIEW_H
#define HISTOGRAMVIEW_H

#include <QWidget>
#include <QtCharts/QChart>

class HistogramView : public QWidget
{
    Q_OBJECT
public:
    using Histogram = QVector<double>;
    explicit HistogramView(QWidget *parent = nullptr);
    Histogram histogram() const;
    QString title() const;

signals:
    void histogramChanged(const Histogram& hist);

public slots:
    void calculateImageHistogram(const QImage& image);
    void setHistogram(const Histogram& hist);
    void setTitle(const QString& title);

private slots:
    void updateChart(const Histogram& hist);

private:
    Histogram data;
    QtCharts::QChart* chart;
};

#endif // HISTOGRAMVIEW_H
