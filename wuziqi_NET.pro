
QT       += core gui
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TEMPLATE = app

CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT
QMAKE_CFLAGS_RELEASE    = -O2
QMAKE_CXXFLAGS_RELEASE  = -O2

HEADERS += \
    mainwindow.h \
    board.h \
    searchplayerdialog.h

SOURCES += \
    mainwindow.cpp \
    main.cpp \
    board.cpp \
    searchplayerdialog.cpp

FORMS += \
    mainwindow.ui \
    getmsgdialog.ui \
    searchplayerdialog.ui
