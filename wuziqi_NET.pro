
QT       += core gui
QT       += network
LIBS += -lWs2_32

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TEMPLATE = app

#QMAKE_CXXFLAGS += -std=c++0x

HEADERS += \
    mainwindow.h \
    board.h \
    searchplayerdialog.h \
    signupdialog.h \
    wzq_msg.h \
    global.h \
    websocket.h \
    messagequeue.h

SOURCES += \
    mainwindow.cpp \
    main.cpp \
    board.cpp \
    searchplayerdialog.cpp \
    signupdialog.cpp \
    wzq_msg.cpp \
    global.cpp \
    websocket.cpp \
    messagequeue.cpp

FORMS += \
    mainwindow.ui \
    getmsgdialog.ui \
    searchplayerdialog.ui \
    signupdialog.ui
