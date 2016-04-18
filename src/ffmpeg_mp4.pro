#-------------------------------------------------
#
# Project created by QtCreator 2016-01-17T09:07:25
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ffmpeg-mp4
TEMPLATE = app

INCLUDEPATH	+=  /opt/ffmpeg-2.8_arm/include
LIBS += -L/opt/ffmpeg-2.8_arm/lib -lavcodec -lavformat -lavutil -lavdevice -lavfilter -lswresample -lswscale

SOURCES += main.cpp\
        basedialog.cpp \
    mvideo.cpp \
    ffmpegvideo.cpp

HEADERS  += basedialog.h \
    mvideo.h \
    ffmpegvideo.h

FORMS    += basedialog.ui \
    mvideo.ui

RESOURCES += \
    skin.qrc

