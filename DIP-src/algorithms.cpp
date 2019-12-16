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
                               double k0, double k1, double k2, double e, int r)
{
    Q_ASSERT_X(image.format()==QImage::Format_Grayscale8,__func__,"Non-grayscale");
    Q_ASSERT_X(k0>=0&&k0<=1&&k1<=k2&&e>=1&&r>=1,__func__,"Invalid parameter");
    QImage out(image.size(),QImage::Format_Grayscale8);
    const int width = image.width();
    const int height = image.height();
    const int size = width*height;
    double g_mean = 0;
    double g_std = 0;
    for (int y=0; y<height; ++y) {
        const uint8_t* line = image.scanLine(y);
        for (int x=0; x<width; ++x) {
            g_mean += line[x];
            g_std += line[x]*line[x];
        }
    }
    g_mean /= size;
    g_std = std::sqrt(g_std/size-g_mean*g_mean); // D(x)=E(x^2)-E(x)^2
    for (int y=0; y<height; ++y) {
        const uint8_t* line = image.scanLine(y);
        uint8_t* out_line = out.scanLine(y);
        for (int x=0; x<width; ++x) {
            double l_mean = 0;
            double l_std = 0;
            uint l_size = 0;
            for (int dy=-r; dy<r; ++dy) {
                const int yy = y+dy;
                if (yy<0 || yy>=height)
                    continue;
                const uint8_t* line = image.scanLine(yy);
                for (int dx=-r; dx<r; ++dx) {
                    const int xx = x+dx;
                    if (xx<0 || x>=width)
                        continue;
                    l_mean += line[xx];
                    l_std += line[xx]*line[xx];
                    l_size += 1;
                }
            }
            l_mean /= l_size;
            l_std = std::sqrt(l_std/l_size-l_mean*l_mean); // D(x)=E(x^2)-E(x)^2
            out_line[x] = l_mean<=k0*g_mean && l_std>=k1*g_std && l_std<=k2*g_std
                        ? static_cast<uint8_t>(std::clamp(e*line[x],0.,1.*UINT8_MAX))
                        : line[x];
        }
    }
    return out;
}
