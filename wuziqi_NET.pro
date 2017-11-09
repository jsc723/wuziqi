
QT       += core gui
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TEMPLATE = app

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
