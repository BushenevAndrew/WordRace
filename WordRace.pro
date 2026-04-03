QT += core gui widgets

CONFIG += c++17

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    gamewidget.cpp

HEADERS += \
    mainwindow.h \
    gamewidget.h \
    wordlistloader.h

DISTFILES += \
    words.txt
