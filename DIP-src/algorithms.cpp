#include "algorithms.h"
#include <cmath>
#include <numeric>

//! 计算图像直方图数据
QVector<double> histogram(const QImage& image)
{
    Q_ASSERT_X(image.format()==QImage::Format_Grayscale8,__func__,"Non-grayscale");
    QVector<double> hist(0x100,0);
    const int width = image.width();
    const int height = image.height();
    for (int y=0; y<height; ++y) {
        const uint8_t* line = image.scanLine(y);
        for (int x=0; x<width; ++x) {
            hist[line[x]] += 1.0/(width*height);
        }
    }
    return hist;
}

//! 直方图均衡化
QImage equalizeHistogram(const QImage& image)
{
    Q_ASSERT_X(image.format()==QImage::Format_Grayscale8,__func__,"Non-grayscale");
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

//! 局部统计增强
QImage localStatisticalEnhance(const QImage &image,
                               double k0, double k1, double k2, double e, uint r)
{
    Q_ASSERT_X(image.format()==QImage::Format_Grayscale8,__func__,"Non-grayscale");
    Q_ASSERT_X(k0>=0&&k0<=1&&k1<=k2&&e>=1&&r>=1,__func__,"Invalid parameter");
    QImage out(image.size(),QImage::Format_Grayscale8);
    out = image;//Do nothing; TODO: implement
    return out;
}
