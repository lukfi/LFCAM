QT       += core gui
CONFIG += console
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = LFCAM
TEMPLATE = app

config += vcwInplace

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += ../Common/System ../Common/Multimedia ../Common/Codecs

HEADERS += \
    ../Common/Utils/BasicWebServer/basicwebserver.h \
    ../Common/Utils/WillowCmdVideo/wcvclientlite.h \
    RequestHandler.h \
    UniDecoder.h \
    VideoController.h

SOURCES += \
    ../Common/Utils/BasicWebServer/basicwebserver.cpp \
    ../Common/Utils/WillowCmdVideo/wcvclientlite.cpp \
    RequestHandler.cpp \
    VideoController.cpp \
    main.cpp

!android {
    CONFIG(debug, debug|release) {
        DESTDIR = $$PWD/../CommonLibs/debug
        LIBS += -L$$PWD/../CommonLibs/debug -lSystem -lMultimedia
        LIBS += -L$$PWD/../CommonLibs/debug/codecs -lH264Decoder
    } else {
        DESTDIR = $$PWD/../CommonLibs/release
        LIBS += -L$$PWD/../CommonLibs/release -lSystem -lMultimedia
        LIBS += -L$$PWD/../CommonLibs/release/codecs -lH264Decoder
    }
}
LIBS += -lssl -lcrypto -lv4l2 -ljpeg -lswscale -lavcodec -lavutil

#vcwInplace {
    INCLUDEPATH += ../Common/Utils/WillowCmdVideo

    HEADERS +=  ../Common/Utils/WillowCmdVideo/imagewindow.h \
                #../Common/Utils/WillowCmdVideo/imagereceiver.h \

    SOURCES +=  ../Common/Utils/WillowCmdVideo/imagewindow.cpp \
                #../Common/Utils/WillowCmdVideo/imagereceiver.cpp \

    DEFINES += WCV_INPLACE
#}
