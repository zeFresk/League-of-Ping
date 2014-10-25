#-------------------------------------------------
#
# Project created by QtCreator 2014-10-25T17:44:35
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = LoLPinger
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h \
    Ping.hpp

FORMS    += mainwindow.ui

QMAKE_CXXFLAGS += -std=c++11 -fpermissive

LIBS += -lwsock32 -pthread
