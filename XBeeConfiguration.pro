# -------------------------------------------------
# Project created by QtCreator 2009-10-17T22:16:07
# -------------------------------------------------
TARGET = XBeeConfiguration

QT       += core gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app
ICON = xbee.icns
SOURCES += main.cpp \
    mainwindow.cpp \
    serialportselector.cpp \
    serialport.cpp \
    serialportposix.cpp \
    serialportwindows.cpp \
    textconsole.cpp \
    xbeecommandlistmodel.cpp \
    xbeecommand.cpp \
    senddialog.cpp \
    hexvalidator.cpp \
    xbeecommandhelper.cpp
HEADERS += mainwindow.h \
    serialportselector.h \
    serialport.h \
    serialportposix.h \
    serialportwindows.h \
    textconsole.h \
    xbeecommandlistmodel.h \
    xbeecommand.h \
    senddialog.h \
    hexvalidator.h \
    xbeecommandhelper.h
FORMS += mainwindow.ui \
    serialportselector.ui \
    senddialog.ui
RESOURCES += xbeecommandresource.qrc \
    icons.qrc
RC_FILE = resource.rc
