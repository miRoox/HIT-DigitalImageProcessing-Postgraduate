#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "algorithms.h"
#include "imageview.h"
#include "histogramview.h"
#include <QCompleter>
#include <QImageReader>
#include <QFileSystemModel>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>

static void setupPathEdit(QLineEdit* edit)
{
    auto completer = new QCompleter(edit);
    auto model = new QFileSystemModel(completer);
    model->setRootPath("");
    completer->setModel(model);
    edit->setCompleter(completer);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupPathEdit(ui->originPathEdit);

    connect(ui->originSelButton,&QPushButton::clicked,
            this,&MainWindow::openImageDialog);
    connect(ui->originPathEdit,&QLineEdit::returnPressed,
            [this]{openImage(ui->originPathEdit->text());});

    connect(ui->originView,&ImageView::imageChanged,
            ui->originHistView,&HistogramView::calculateImageHistogram);

    connect(ui->originView,&ImageView::saveImageTriggered,
            this,&MainWindow::saveImageFromView);
    connect(ui->globalEnhView,&ImageView::saveImageTriggered,
            this,&MainWindow::saveImageFromView);
    connect(ui->localEnhView,&ImageView::saveImageTriggered,
            this,&MainWindow::saveImageFromView);
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
        ui->originView->setImage(image.convertToFormat(QImage::Format_Grayscale8));
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
        ui->originPathEdit->setText(fileName);
        openImage(fileName);
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
