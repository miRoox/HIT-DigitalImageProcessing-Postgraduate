#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <QVector>
#include <QImage>

QVector<double> histogram(const QImage& image);
QImage histogramEqualize(const QImage& image);

#endif // ALGORITHMS_H
