QT += core gui network widgets

CONFIG += c++17

TARGET = defense-edu
TEMPLATE = app

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/aiservicemanager.cpp \
    src/ollamamanager.cpp \
    src/settingsmanager.cpp \
    src/timelinewidget.cpp \
    src/gamewidget.cpp \
    src/firstrundialog.cpp \
    src/tutorialdialog.cpp

HEADERS += \
    src/mainwindow.h \
    src/aiservicemanager.h \
    src/ollamamanager.h \
    src/settingsmanager.h \
    src/timelinewidget.h \
    src/gamewidget.h \
    src/firstrundialog.h \
    src/tutorialdialog.h

RESOURCES += \
    resources.qrc

# Windows specific
win32 {
    RC_FILE = resources/app.rc
}

# MinGW static linking for runtime libs
QMAKE_LFLAGS += -static-libgcc -static-libstdc++
