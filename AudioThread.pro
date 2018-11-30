#-------------------------------------------------
#
# Project created by QtCreator 2018-11-02T13:07:00
#
#-------------------------------------------------

QT       += network core gui

TARGET = $$qtLibraryTarget(AudioThread)
TEMPLATE = lib

DEFINES += AUDIOTHREAD_LIBRARY

DESTDIR = dist
# Версия библиотеки.
VERSION = 0.1.0
# Указываем, что собирать надо release и debug версии библиотек
CONFIG += build_all

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

LIBS += -L"$$_PRO_FILE_PWD_/libs/" -lbass -ltags -lbass_fx -lbass_aac -lbassalac -lbassflac -lbasshls -lbasswasapi -lbasswma -lbasswv

SOURCES += \
        audiothread.cpp \
    pixmaploader.cpp \
    track.cpp

HEADERS += \
        audiothread.h \
        audiothread_global.h \ 
    bass.h \
    pixmaploader.h \
    tags.h \
    track.h \
    bass_aac.h \
    bassalac.h \
    bassflac.h \
    basshls.h \
    basswasapi.h \
    basswma.h \
    basswv.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
