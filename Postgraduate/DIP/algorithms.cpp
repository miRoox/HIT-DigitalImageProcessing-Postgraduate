#include "algorithms.h"
#include <cmath>
#include <numeric>

//! 计算图像直方图数据
QVector<double> histogram(const QImage& image)
{
    const QImage img(image.convertToFormat(QImage::Format_Grayscale8));
    QVector<double> hist(0x100,0);
    const int width = img.width();
    const int height = img.height();
    for (int y=0; y<height; ++y) {
        const uint8_t* line = img.scanLine(y);
        for (int x=0; x<width; ++x) {
            hist[line[x]] += 1.0/(width*height);
        }
    }
    return hist;
}

//! 直方图均衡化
QImage histogramEqualize(const QImage& image)
{
    QImage out(image.size(),QImage::Format_Grayscale8);
    QVector<double> hist_map = histogram(image);
    std::partial_sum(hist_map.cbegin(),hist_map.cend(),hist_map.begin());
    for (int y=0; y<image.height(); ++y) {
        const uint8_t* line = image.scanLine(y);
        uint8_t* out_line = out.scanLine(y);
        for (int x=0; x<image.width(); ++x) {
            out_line[x] = static_cast<uint8_t>(std::lround(0xff*hist_map[line[x]]));
        }
    }
    return out;
}
