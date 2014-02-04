#-------------------------------------------------
#
# Project created by QtCreator 2013-07-22T11:51:36
#
#-------------------------------------------------

QT       += core network

QT       -= gui

TARGET = http-daemon
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
	httpparser.cpp \
	httpconnection.cpp \
	httpconnectionmanager.cpp \
	dispatcher.cpp \
	filehttprequesthandler.cpp \
	dirhttprequesthandler.cpp \
	httpdaemon.cpp

HEADERS += \
	httpparser.h \
	httpconnection.h \
	httpconnectionmanager.h \
	dispatcher.h \
	filehttprequesthandler.h \
	dirhttprequesthandler.h \
	httpdaemon.h

OTHER_FILES +=

#TODO: find a way to link to a more general location, if I move on another machine I won't have to create all the build-* dirs
#TODO: find a way to move the plugins to their central location without relying on qt creator

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../build-http-daemon-components-Desktop-Debug/release/ -lhttp-daemon-components
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build-http-daemon-components-Desktop-Debug/debug/ -lhttp-daemon-components
else:unix: LIBS += -L$$PWD/../build-http-daemon-components-Desktop-Debug/ -lhttp-daemon-components

INCLUDEPATH += $$PWD/../http-daemon-components
DEPENDPATH += $$PWD/../http-daemon-components

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../build-http-daemon-components-Desktop-Debug/release/http-daemon-components.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../build-http-daemon-components-Desktop-Debug/debug/http-daemon-components.lib
else:unix: PRE_TARGETDEPS += $$PWD/../build-http-daemon-components-Desktop-Debug/libhttp-daemon-components.a
