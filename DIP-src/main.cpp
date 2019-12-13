#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    w.openImage(":/rc/test-images/tungsten.tif");
    return a.exec();
}
