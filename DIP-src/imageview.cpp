#include "imageview.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QAction>
#include <QClipboard>
#include <QApplication>

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
    setContextMenuPolicy(Qt::ActionsContextMenu);
    connect(actionCopy,&QAction::triggered,[this]{
        QApplication::clipboard()->setImage(current);
    });
    connect(actionSave,&QAction::triggered,
            [this]{emit saveImageTriggered(current);});
    view->setPixmap(QPixmap(":/rc/icon/no-image.png"));
    setDisabled(true); // disabled when no image
}

void ImageView::setImage(const QImage &image)
{
    if (image == current || image.isNull())
        return;
    current = image;
    setEnabled(true);
    view->setPixmap(QPixmap::fromImage(current));
    emit imageChanged(current);
}

QImage ImageView::image() const
{
    return current;
}
