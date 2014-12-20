#-------------------------------------------------
#
# Project created by QtCreator 2014-12-20T23:02:58
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = fiddle
TEMPLATE = app


SOURCES += main.cpp\
        fiddlewindow.cpp

HEADERS  += fiddlewindow.h \
    cvimagewidget.h

FORMS    += fiddlewindow.ui

LIBS     += `pkg-config opencv --libs`
