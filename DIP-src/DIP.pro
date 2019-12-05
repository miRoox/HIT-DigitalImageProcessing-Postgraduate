# 数字图像处理

TEMPLATE = app
TARGET = DIP

CONFIG(release, debug|release):  DESTDIR = ../bin

VERSION = 0.5.0

QT       += core gui charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
mac: CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    algorithms.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    algorithms.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

RESOURCES += \
    asset.qrc

#CONFIG(release, debug|release) {
#      win32: QMAKE_POST_LINK += $$[QT_INSTALL_BINS]/windeployqt $$shell_path($$absolute_path($$DESTDIR))  $$escape_expand(\\n\\t)
#      mac: QMAKE_POST_LINK += $$[QT_INSTALL_BINS]/macdeployqt $$shell_path($$absolute_path($$DESTDIR))  $$escape_expand(\\n\\t)
#}
