#-------------------------------------------------
#
# Project created by QtCreator 2013-07-22T11:51:36
#
#-------------------------------------------------

QT       += core network concurrent

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
	httpdaemon.cpp \
	logging.cpp \
	configuration.cpp

HEADERS += \
	httpparser.h \
	httpconnection.h \
	httpconnectionmanager.h \
	dispatcher.h \
	httpdaemon.h \
	logging.h \
	configuration.h

QMAKE_CXXFLAGS += -std=c++11

OTHER_FILES += \
	httpdaemon.conf

DEFINES += CONFIG_DIR=\\\"/path/to/http-daemon/config/dir/\\\"

CONFIG(release, debug|release){
	DEFINES += QT_NO_DEBUG_OUTPUT QT_NO_WARNING_OUTPUT
	message("Release mode: No debug or warning messages from Qt")
}

#TODO: find a way to link to a more general location, if I move on another machine I won't have to create all the build-* dirs
#TODO: find a way to move the plugins to their central location without relying on qt creator
# provide an installation script

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../build-http-daemon-components-Desktop-Debug/release/ -lhttp-daemon-components
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build-http-daemon-components-Desktop-Debug/debug/ -lhttp-daemon-components
else:unix: LIBS += -L$$PWD/../build-http-daemon-components-Desktop-Debug/ -lhttp-daemon-components

INCLUDEPATH += $$PWD/../http-daemon-components
DEPENDPATH += $$PWD/../http-daemon-components

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../build-http-daemon-components-Desktop-Debug/release/libhttp-daemon-components.a
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../build-http-daemon-components-Desktop-Debug/debug/libhttp-daemon-components.a
else:unix: PRE_TARGETDEPS += $$PWD/../build-http-daemon-components-Desktop-Debug/libhttp-daemon-components.a
