#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "algorithms.h"
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QAreaSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChartView>
#include <QPointF>
#include <QToolButton>
#include <QMenu>
#include <QImageReader>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>

using namespace QtCharts;

constexpr static int GraphicsItemImageDataKey = 0;

static QVector<QPointF> linePointsFromHistogram(const QVector<double>& hist)
{
    QVector<QPointF> points;
    points.reserve(0x100*2);
    for (int i=0;i<0x100;++i)
    {
        points << QPointF{1.0*i,hist[i]} << QPointF{1.0*(i+1),hist[i]};
    }
    return points;
}

static void updateHistogramChart(QChart*const chart, const QVector<double>& hist)
{
    auto series = qobject_cast<QAreaSeries*>(chart->series().first());
    Q_CHECK_PTR(series);
    auto line = series->upperSeries();
    line->replace(linePointsFromHistogram(hist));
    chart->axes(Qt::Vertical).first()->setMax(*std::max_element(hist.begin(),hist.end()));
}

static void setupHistogramView(QChartView*const view, QChart*const chart)
{
    QVector<double> hist(0x100,0);
    auto lseries = new QLineSeries();
    lseries->append(linePointsFromHistogram(hist).toList());
    auto series = new QAreaSeries(lseries);
    QPen pen(Qt::gray);
    pen.setWidth(1);
    series->setPen(pen);

    QLinearGradient gradient(QPointF(0, 0), QPointF(1, 0));
    gradient.setColorAt(0.0, Qt::black);
    gradient.setColorAt(1.0, Qt::white);
    gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
    series->setBrush(gradient);

    chart->addSeries(series);
    chart->legend()->hide();
    chart->createDefaultAxes();
    chart->axes(Qt::Horizontal).first()->setRange(0, 256);
    chart->axes(Qt::Vertical).first()->setMin(0);

    view->setChart(chart);
    view->setRenderHint(QPainter::Antialiasing);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    { // “打开”按钮
        auto openBtn = new QToolButton;
        openBtn->setIcon(QIcon(":/rc/icon/open.png"));
        auto menu = new QMenu;
        // TODO: 自带测试图片
        auto openFileAction = new QAction(tr("从文件中打开"));
        connect(openFileAction,&QAction::triggered,
                this,&MainWindow::openImageDialog);
        menu->addAction(openFileAction);
        openBtn->setMenu(menu);
        openBtn->setPopupMode(QToolButton::MenuButtonPopup);
        ui->toolBar->addWidget(openBtn);
    }
    // TODO:保存、关于

    { // 原图像视图
        auto scene = new QGraphicsScene(this);
        auto item = scene->addPixmap(QPixmap(":/rc/icon/no-image.png"));
        connect(this,&MainWindow::imageLoaded,[item](const QImage& image){
            item->setPixmap(QPixmap::fromImage(image));
            item->setData(GraphicsItemImageDataKey,image);
        });
        ui->originView->setScene(scene);// TODO: 缩放
    }

    { // 原图像直方图视图
        auto chart = new QChart;
        setupHistogramView(ui->originHistView,chart);
        chart->setTitle(tr("原图像的灰度直方图"));
        connect(this,&MainWindow::imageLoaded,[chart](const QImage& image){
            updateHistogramChart(chart,histogram(image));
        });
    }
    // TODO: 直方图均衡化、局部直方图统计增强

    setWindowTitle(tr("数字图像处理 - 作业"));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openImage(const QString &fileName)
{
    QImageReader reader(fileName);
    QImage image;
    if (reader.read(&image)) {
        setWindowFilePath(fileName);
        if (image.format()!=QImage::Format_Grayscale8) {
            ui->statusbar->showMessage(tr("警告：图像已被转换为灰度格式"),10000);
        }
        emit imageLoaded(image.convertToFormat(QImage::Format_Grayscale8));
    } else {
        QMessageBox::critical(this,tr("读取图像失败！"),reader.errorString(),QMessageBox::Cancel);
        ui->statusbar->showMessage(tr("读取图像失败！"),5000);
    }
}

void MainWindow::openImageDialog()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("打开图像"),".",
                                                    tr("图片 (*.png *.jpg *.jpeg *.bmp *.xpm)"));
    if (!fileName.isEmpty()) {
        openImage(fileName);//TODO: 允许拖放打开
    }
}

void MainWindow::saveImageFromView(const QImage &image)
{
    QString key = QFileInfo(window()->windowFilePath()).baseName();
    QString fileName = QFileDialog::getSaveFileName(this,tr("保存图像"),
                                                    key + ".out.png",
                                                    tr("图片 (*.png *.jpg *.jpeg *.bmp *.xpm)"));
    if (!fileName.isEmpty())
    {
        bool retry = false;
        do {
            if (image.save(fileName))
            {
                QMessageBox::information(this,tr("图像保存成功！"),
                                         tr("图像成功保存到 %1。").arg(fileName),
                                         QMessageBox::Ok);
                retry = false;
            }
            else
            {
                ui->statusbar->showMessage(tr("图像保存失败！"));
                auto button =
                        QMessageBox::warning(this,tr("图像保存失败！"),
                                             tr("是否重试？"),
                                             QMessageBox::Retry,QMessageBox::Cancel);
                retry = button==QMessageBox::Retry;
                ui->statusbar->clearMessage();
            }
        }
        while(retry);
    }
}
