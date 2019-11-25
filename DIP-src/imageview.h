#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <QWidget>
#include <QImage>

class QLabel;

class ImageView : public QWidget
{
    Q_OBJECT
public:
    explicit ImageView(QWidget *parent = nullptr);
    QImage image() const;

signals:
    void imageChanged(const QImage& image);
    void saveImageTriggered();

public slots:
    void setImage(const QImage& image);

private:
    QLabel* view;
    QImage current;
};

#endif // IMAGEVIEW_H
