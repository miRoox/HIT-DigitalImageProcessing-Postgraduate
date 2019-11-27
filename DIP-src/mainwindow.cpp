#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "algorithms.h"
#include <QImageReader>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
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
//        ui->originView->setImage(image.convertToFormat(QImage::Format_Grayscale8));
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
