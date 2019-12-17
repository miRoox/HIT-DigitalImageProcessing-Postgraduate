#include "algorithms.h"
#include <cmath>
#include <numeric>
#include <tuple>

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
    // $s_k=T\qty(r_k)=(L-1)\sum_{j=0}^k p_r(r_j)$
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
    auto evalMeanStd = [=,&image](int xl, int xh, int yl, int yh)->auto {
        uint size = 0;
        double mean = 0;
        double mean2 = 0;
        for (int y=yl; y<=yh; ++y) {
            if (y<0 || y>=height)
                continue;
            const uint8_t* line = image.scanLine(y);
            for (int x=xl; x<=xh; ++x) {
                if (x<0 || x>=width)
                    continue;
                size  += 1;
                mean  += line[x];
                mean2 += line[x]*line[x];
            }
        }
        mean  /= size;
        mean2 /= size;
        double stdv = std::sqrt(mean2-mean*mean); // $D(x)=E(x^2)-E(x)^2$
        return std::make_tuple(mean,stdv);
    };
    const auto [gMean, gStd] = evalMeanStd(0, width-1, 0, height-1);
    for (int y=0; y<height; ++y) {
        const uint8_t* line = image.scanLine(y);
        uint8_t* outLine = out.scanLine(y);
        for (int x=0; x<width; ++x) {
            const auto [lMean, lStd] = evalMeanStd(x-r, x+r, y-r, y+r);
            outLine[x] = lMean<=k0*gMean && lStd>=k1*gStd && lStd<=k2*gStd
                        ? static_cast<uint8_t>(std::clamp(e*line[x],0.,1.*UINT8_MAX))
                        : line[x];
        }
    }
    return out;
}
