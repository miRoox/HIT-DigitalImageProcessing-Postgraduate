#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "imageview.h"
#include <QImageReader>
#include <QFileDialog>
#include <QMessageBox>
#include <QtCharts>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->originSelButton,&QPushButton::clicked,
            this,&MainWindow::openImageDialog);
    connect(ui->originPathEdit,&QLineEdit::returnPressed,
            [this]{openImage(ui->originPathEdit->text());});
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
        ui->originView->setImage(image);
    } else {
        QMessageBox::critical(this,tr("无法读取图像"),reader.errorString(),QMessageBox::Cancel);
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

