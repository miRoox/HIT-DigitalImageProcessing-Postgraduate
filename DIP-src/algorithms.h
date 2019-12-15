#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <QVector>
#include <QImage>

QVector<double> histogram(const QImage& image);
QImage equalizeHistogram(const QImage& image);
QImage localStatisticalEnhance(const QImage& image,
                               double k0, double k1, double k2, double E, uint r=1);

#endif // ALGORITHMS_H
