#include "imageview.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QAction>
#include <QClipboard>
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>

ImageView::ImageView(QWidget *parent)
    : QWidget(parent), view(new QLabel), current()
{
    auto layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->addWidget(view);
    setLayout(layout);
    auto actionCopy = new QAction(tr("&Copy"),this);
    auto actionSave = new QAction(tr("&Save"),this);
    addAction(actionCopy);
    addAction(actionSave);
    connect(actionCopy,&QAction::triggered,[this]{
        if (current.isNull())
            return;
        QApplication::clipboard()->setImage(current);
    });
    connect(actionSave,&QAction::triggered,this,&ImageView::saveImage);
    view->setPixmap(QPixmap(":/rc/icon/no-image.png"));
}

void ImageView::setImage(const QImage &image)
{
    if (image == current)
        return;
    current = image;
    view->setPixmap(QPixmap::fromImage(current));
    emit imageChanged(current);
}

void ImageView::saveImage()
{
    if (current.isNull())
    {
        QMessageBox::information(this,tr("没有可保存的图像！"),
                                 tr("没有可供保存的图像，请先载入原图像。"),
                                 QMessageBox::Cancel);
        return;
    }
    QString key = window()->windowTitle();
    QString fileName = QFileDialog::getSaveFileName(this,tr("保存图像"),
                                                    key + ".out.png",
                                                    tr("图像 (*.png *.jpg *.jpeg *.bmp *.xpm)"));
    if (!fileName.isEmpty())
    {
        bool retry = false;
        do {
            if (current.save(fileName))
            {
                QMessageBox::information(this,tr("图像保存成功！"),
                                         tr("图像成功保存到 %1。").arg(fileName),
                                         QMessageBox::Ok);
                retry = false;
            }
            else
            {
                auto button =
                        QMessageBox::warning(this,tr("图像保存失败！"),
                                             tr("是否重试？"),
                                             QMessageBox::Retry,QMessageBox::Cancel);
                retry = button==QMessageBox::Retry;
            }
        }
        while(retry);
    }
}

QImage ImageView::image() const
{
    return current;
}
