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
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>

using namespace QtCharts;

static constexpr int originTabIndex = 0;
static constexpr int globalEnhTabIndex = 1;
static constexpr int localEnhTabIndex = 2;
static constexpr int tabCount = 3;

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
    , ui(new Ui::MainWindow),
      images(tabCount)
{
    ui->setupUi(this);

    { // “打开”按钮
        auto openBtn = new QToolButton;
        openBtn->setIcon(QIcon(":/rc/icon/open.png"));
        auto menu = new QMenu;
        for (const QFileInfo& file : QDir(":/rc/test-images/").entryInfoList()) {
            menu->addAction(file.baseName(),[this,file]{
                openImage(file.absoluteFilePath());
            });
        }
        menu->addSeparator();
        menu->addAction(tr("从文件中打开"),[this]{
            QString fileName = QFileDialog::getOpenFileName(this,tr("打开图像"),".",
                                                            tr("图片 (*.png *.jpg *.jpeg *.bmp *.xpm)"));
            if (!fileName.isEmpty()) {
                openImage(fileName);
            }
        });
        openBtn->setMenu(menu);
        openBtn->setPopupMode(QToolButton::MenuButtonPopup);
        ui->toolBar->addWidget(openBtn);
    }

    { // “保存”按钮
        auto saveBtn = new QToolButton;
        saveBtn->setIcon(QIcon(":/rc/icon/save.png"));
        auto menu = new QMenu;
        menu->addAction(tr("保存当前图片"),[this]{
            QString fileName = QFileDialog::getSaveFileName(this,tr("保存当前图片"),
                                                            QFileInfo(windowFilePath()).fileName(),
                                                            tr("图片 (*.png *.jpg *.jpeg *.bmp *.xpm)"));
            saveImage(images[ui->tabWidget->currentIndex()],fileName);
        });
        menu->addAction(tr("保存当前直方图"),[this]{
            QGraphicsView* current = nullptr;
            const auto list = {ui->originHistView,ui->globalEnhHistView,ui->localEnhHistView};
            for (const auto view : list) {
                if (ui->tabWidget->currentWidget()->isAncestorOf(view)) {
                    current = view;
                    break;
                }
            }
            Q_CHECK_PTR(current);
            QImage image(current->rect().size(),QImage::Format_Grayscale8);
            QPainter painter(&image);
            current->render(&painter);
            QFileInfo fileInfo(window()->windowFilePath());
            QString path = fileInfo.baseName()+"-hist."+fileInfo.suffix();
            QString fileName = QFileDialog::getSaveFileName(this,tr("保存当前直方图"),path,
                                                            tr("图片 (*.png *.jpg *.jpeg *.bmp *.xpm)"));
            saveImage(image,fileName);
        });
        saveBtn->setMenu(menu);
        saveBtn->setPopupMode(QToolButton::MenuButtonPopup);
        ui->toolBar->addWidget(saveBtn);
    }

    { // “关于”按钮
        ui->toolBar->addAction(QIcon(":/rc/icon/information.png"),
                               tr("关于"),[this]{
            QMessageBox::about(this,tr("关于本程序"),
                               tr("<h1>数字图像处理 - 作业</h1>")+
                               tr("<h2>局部统计特征增强</h2>")+
                               tr("<p>并与全局直方图均衡化对比，用钨丝灰度图。</p>")+
                               tr("<hr/>")+
                               tr("<p>本程序基于<a href=\"https://www.qt.io/\">Qt</a>构建</p>")+
                               tr("<p>图标由<a href=\"https://www.flaticon.com/authors/good-ware\" title=\"Good Ware\">Good Ware</a>和<a href=\"https://www.flaticon.com/authors/freepik\" title=\"Freepik\">Freepik</a>制作</p>"));
        });
    }

    setAcceptDrops(true);
    setWindowTitle(tr("数字图像处理 - 作业"));

    { // Splitter 同步
        auto syncSplitter = [this](const QSplitter* current)->auto{
            return [current,this]{
                ui->splitter_1->setSizes(current->sizes());
                ui->splitter_2->setSizes(current->sizes());
                ui->splitter_3->setSizes(current->sizes());
            };
        };
        connect(ui->splitter_1,&QSplitter::splitterMoved,syncSplitter(ui->splitter_1));
        connect(ui->splitter_2,&QSplitter::splitterMoved,syncSplitter(ui->splitter_2));
        connect(ui->splitter_3,&QSplitter::splitterMoved,syncSplitter(ui->splitter_3));
    }

    { // 原图像视图
        connect(this,&MainWindow::imageLoaded,[this](const QImage& image){
            images.replace(originTabIndex,image);
        });
        auto scene = new QGraphicsScene(this);
        auto item = scene->addPixmap(QPixmap(":/rc/icon/no-image.png"));
        connect(this,&MainWindow::imageLoaded,[item](const QImage& image){
            item->setPixmap(QPixmap::fromImage(image));
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
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::openImage(const QString &fileName)
{
    QImageReader reader(fileName);
    QImage image;
    if (reader.read(&image)) {
        setWindowFilePath(fileName);
        if (image.format()!=QImage::Format_Grayscale8) {
            ui->statusbar->showMessage(tr("警告：图像已被转换为灰度格式"),10000);
        }
        emit imageLoaded(image.convertToFormat(QImage::Format_Grayscale8));
        return true;
    } else {
        QMessageBox::critical(this,tr("读取图像失败！"),reader.errorString(),QMessageBox::Cancel);
        ui->statusbar->showMessage(tr("读取图像失败！"),5000);
        return false;
    }
}

void MainWindow::saveImage(const QImage &image, const QString &fileName)
{
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

void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls()) {
        e->acceptProposedAction();
    } else {
        e->ignore();
    }
}

void MainWindow::dropEvent(QDropEvent *e)
{
    const auto mimeData = e->mimeData();
    if (mimeData->hasUrls()) {
        QString fileName = mimeData->urls().first().toLocalFile();
        if (!fileName.isEmpty()) {
            if (openImage(fileName)){
                ui->tabWidget->setCurrentWidget(ui->originTab);
            }
        }
    }
}
