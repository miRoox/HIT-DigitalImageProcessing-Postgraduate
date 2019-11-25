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
    auto actionCopy = new QAction(tr("复制(&C)"),this);
    auto actionSave = new QAction(tr("保存(&S)"),this);
    addAction(actionCopy);
    addAction(actionSave);
    connect(actionCopy,&QAction::triggered,[this]{
        if (current.isNull())
            return;
        QApplication::clipboard()->setImage(current);
    });
    connect(actionSave,&QAction::triggered,this,&ImageView::saveImageTriggered);
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

QImage ImageView::image() const
{
    return current;
}
