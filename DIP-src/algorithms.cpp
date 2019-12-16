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
    QVector<double> histMap = histogram(image);
    std::partial_sum(histMap.cbegin(),histMap.cend(),histMap.begin());
    std::transform(histMap.cbegin(),histMap.cend(),histMap.begin(),
                   [](double c)->double{return std::lround(UINT8_MAX*c);});
    for (int y=0; y<image.height(); ++y) {
        const uint8_t* line = image.scanLine(y);
        uint8_t* outLine = out.scanLine(y);
        for (int x=0; x<image.width(); ++x) {
            outLine[x] = static_cast<uint8_t>(histMap[line[x]]);
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
    double gMean = 0;
    double gMean2 = 0;
    for (int y=0; y<height; ++y) {
        const uint8_t* line = image.scanLine(y);
        for (int x=0; x<width; ++x) {
            gMean += line[x];
            gMean2 += line[x]*line[x];
        }
    }
    gMean /= size;
    gMean2 /= size;
    double gStd = std::sqrt(gMean2-gMean*gMean); // D(x)=E(x^2)-E(x)^2
    for (int y=0; y<height; ++y) {
        const uint8_t* line = image.scanLine(y);
        uint8_t* outLine = out.scanLine(y);
        for (int x=0; x<width; ++x) {
            double lMean = 0;
            double lMean2 = 0;
            uint lSize = 0;
            for (int dy=-r; dy<r; ++dy) {
                const int yy = y+dy;
                if (yy<0 || yy>=height)
                    continue;
                const uint8_t* line = image.scanLine(yy);
                for (int dx=-r; dx<r; ++dx) {
                    const int xx = x+dx;
                    if (xx<0 || x>=width)
                        continue;
                    lMean += line[xx];
                    lMean2 += line[xx]*line[xx];
                    lSize += 1;
                }
            }
            lMean /= lSize;
            lMean2 /= lSize;
            double lStd = std::sqrt(lMean2-lMean*lMean); // D(x)=E(x^2)-E(x)^2
            outLine[x] = lMean<=k0*gMean && lStd>=k1*gStd && lStd<=k2*gStd
                        ? static_cast<uint8_t>(std::clamp(e*line[x],0.,1.*UINT8_MAX))
                        : line[x];
        }
    }
    return out;
}
