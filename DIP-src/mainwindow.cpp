#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "labeledslider.h"
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
#include <QWidgetAction>
#include <QMenu>
#include <QFormLayout>
#include <QImageReader>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QApplication>
#include <QClipboard>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>

using namespace QtCharts;

static constexpr double zoomRatio = 1.025;

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

static QImage renderGraphicsView(QGraphicsView*const view)
{
    QImage image(view->rect().size(),QImage::Format_ARGB32);
    QPainter painter(&image);
    view->render(&painter);
    return image;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    { // “打开”按钮
        auto openBtn = new QToolButton;
        openBtn->setText(tr("打开"));
        openBtn->setToolTip(tr("打开"));
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
        openBtn->setPopupMode(QToolButton::InstantPopup);
        ui->toolBar->addWidget(openBtn);
    }

    { // “保存”按钮
        auto saveBtn = new QToolButton;
        saveBtn->setText(tr("保存"));
        saveBtn->setToolTip(tr("保存"));
        saveBtn->setIcon(QIcon(":/rc/icon/save.png"));
        auto menu = new QMenu;
        menu->addAction(tr("保存当前图片"),[this]{
            QString fileName = QFileDialog::getSaveFileName(this,tr("保存当前图片"),
                                                            QFileInfo(windowFilePath()).fileName(),
                                                            tr("图片 (*.png *.jpg *.jpeg *.bmp *.xpm)"));
            QImage images[] = {origin,globalEnh,localEnh};// 注意：Tab页顺序依赖
            saveImage(images[ui->tabWidget->currentIndex()],fileName);
        });
        menu->addAction(tr("保存当前直方图"),[this]{
            QChartView* views[] = {ui->originHistView,ui->globalEnhHistView,ui->localEnhHistView};
            auto current = views[ui->tabWidget->currentIndex()];// 注意：Tab页顺序依赖
            QImage image = renderGraphicsView(current);
            QFileInfo fileInfo(window()->windowFilePath());
            QString path = fileInfo.baseName()+"-hist."+fileInfo.suffix();
            QString fileName = QFileDialog::getSaveFileName(this,tr("保存当前直方图"),path,
                                                            tr("图片 (*.png *.jpg *.jpeg *.bmp *.xpm)"));
            saveImage(image,fileName);
        });
        saveBtn->setMenu(menu);
        saveBtn->setPopupMode(QToolButton::InstantPopup);
        ui->toolBar->addWidget(saveBtn);
    }

    { // 参数设置
        auto settingBtn = new QToolButton;
        settingBtn->setText(tr("参数设置"));
        settingBtn->setToolTip(tr("参数设置"));
        settingBtn->setIcon(QIcon(":/rc/icon/settings"));
        auto settingAction = ui->toolBar->addWidget(settingBtn);
        settingAction->setVisible(false);
        connect(ui->tabWidget,&QTabWidget::currentChanged,[settingAction,this]{
           settingAction->setVisible(ui->tabWidget->currentWidget()==ui->localEnhTab);
        });
        auto waction = new QWidgetAction(settingBtn);
        auto panel = new QWidget;
        auto panelLayout = new QFormLayout(panel);
        auto k0Slider = new LabeledSlider(0,1,0.05);
        panelLayout->addRow(tr("k&0:"),k0Slider);
        auto k1Slider = new LabeledSlider(0,1,0.05);
        panelLayout->addRow(tr("k&1:"),k1Slider);
        auto k2Slider = new LabeledSlider(0,1,0.05);
        panelLayout->addRow(tr("k&2:"),k2Slider);
        auto mSlider = new LabeledSlider(1,10,0.5);
        panelLayout->addRow(tr("&m:"),mSlider);
        connect(k1Slider,&LabeledSlider::valueChanged,[k1Slider,k2Slider](double d){
            if (d>=k2Slider->value())
                k1Slider->setValue(k2Slider->value());
        });
        connect(k2Slider,&LabeledSlider::valueChanged,[k1Slider,k2Slider](double d){
            if (d<=k1Slider->value())
                k2Slider->setValue(k1Slider->value());
        });
        waction->setDefaultWidget(panel);
        auto menu = new QMenu;
        menu->addAction(waction);
        settingBtn->setMenu(menu);
        settingBtn->setPopupMode(QToolButton::InstantPopup);
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

    // 视图设置模板
    auto setupViews = [this](
            const QImage& image,
            QGraphicsView* imageView,
            QChartView* histView,
            const QString& histTitle,
            void (MainWindow::*signal)()){
        auto addActionTo = [](QWidget* widget)->auto{
            return [widget](
                    const QString& text,
                    const QKeySequence& shortcut,
                    auto&& functor){
                auto action = new QAction(text,widget);
                action->setShortcut(shortcut);
                action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
                widget->connect(action,&QAction::triggered,functor);
                widget->addAction(action);
            };
        };
        { // 图像视图
            auto scene = new QGraphicsScene(this);
            auto item = scene->addPixmap(QPixmap(":/rc/icon/no-image.png"));
            connect(this,signal,[item,&image]{
                item->setPixmap(QPixmap::fromImage(image));
            });
            imageView->setScene(scene);
            imageView->setResizeAnchor(QGraphicsView::AnchorUnderMouse);
            auto addActionToView = addActionTo(imageView);
            addActionToView(tr("复制"),QKeySequence::Copy,[&image]{
                QApplication::clipboard()->setImage(image);
            });
            addActionToView(tr("放大"),QKeySequence::ZoomIn,[imageView]{
                imageView->scale(zoomRatio,zoomRatio);
            });
            addActionToView(tr("缩小"),QKeySequence::ZoomOut,[imageView]{
                imageView->scale(1/zoomRatio,1/zoomRatio);
            });
            addActionToView(tr("恢复实际大小"),QKeySequence(Qt::CTRL|Qt::Key_1),[imageView]{
                imageView->resetTransform();
            });
            imageView->setContextMenuPolicy(Qt::ActionsContextMenu);
            imageView->installEventFilter(this);
        }
        { // 直方图视图
            auto chart = new QChart;
            setupHistogramView(histView,chart);
            addActionTo(histView)(tr("复制"),QKeySequence::Copy,[histView]{
                QApplication::clipboard()->setImage(renderGraphicsView(histView));
            });
            histView->setContextMenuPolicy(Qt::ActionsContextMenu);
            chart->setTitle(histTitle);
            connect(this,signal,[chart,&image]{
                updateHistogramChart(chart,histogram(image));
            });
        }
    };

    setupViews(origin,ui->originView,ui->originHistView,tr("原图像的灰度直方图"),&MainWindow::imageLoaded);
    setupViews(globalEnh,ui->globalEnhView,ui->globalEnhHistView,tr("全局直方图均衡化图像的灰度直方图"),&MainWindow::globalEnhUpdated);
    // TODO: 局部直方图统计增强

    connect(this,&MainWindow::imageLoaded,[this]{
        globalEnh = equalizeHistogram(origin);
        emit globalEnhUpdated();
    });
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
        origin = image.convertToFormat(QImage::Format_Grayscale8);
        emit imageLoaded();
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

static bool viewZoomByWheel(QGraphicsView* view, QWheelEvent* e)
{
    if (QApplication::keyboardModifiers() == Qt::ControlModifier)
    {
        if (e->delta() > 0) {
            view->scale(zoomRatio,zoomRatio);
        } else {
            view->scale(1/zoomRatio,1/zoomRatio);
        }
        return true;
    }
    return false;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *e)
{
    auto view = qobject_cast<QGraphicsView*>(obj);
    if (view) {
        if (e->type()==QEvent::Wheel) {
            return viewZoomByWheel(view,static_cast<QWheelEvent*>(e));
        }
    }
    return QMainWindow::eventFilter(obj,e);
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
