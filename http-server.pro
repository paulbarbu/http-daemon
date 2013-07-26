#-------------------------------------------------
#
# Project created by QtCreator 2013-07-22T11:51:36
#
#-------------------------------------------------

QT       += core network

QT       -= gui

TARGET = http-server
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    httpserver.cpp \
    httpparser.cpp \
    httpconnection.cpp \
    httpconnectionmanager.cpp

HEADERS += \
    httpserver.h \
    httpparser.h \
    httpconnection.h \
    httpconnectionmanager.h

OTHER_FILES +=
