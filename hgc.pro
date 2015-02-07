#-------------------------------------------------
#
# Project created by QtCreator 2015-02-06T13:32:40
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = hgc
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    filedialog.cpp

HEADERS  += mainwindow.h \
    filedialog.h

FORMS    += mainwindow.ui \
    filedialog.ui

DEFINES += FAST_PERFORMANCE
