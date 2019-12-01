#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void imageLoaded(const QImage& image);

public slots:
    bool openImage(const QString& fileName);
    void saveImage(const QImage& image,const QString& fileName);

protected:
    void dragEnterEvent(QDragEnterEvent* e);
    void dropEvent(QDropEvent* e);

private:
    Ui::MainWindow *ui;
    QVector<QImage> images;
};
#endif // MAINWINDOW_H
