#-------------------------------------------------
#
# Project created by QtCreator 2018-01-03T18:52:15
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = vehicles_tracking
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    sources/vehicle.cpp \
    sources/box2.cpp

HEADERS  += mainwindow.h \
    sources/vehicle.h \
    sources/box2.h

FORMS    += mainwindow.ui

INCLUDEPATH += sources \
    sources/darknet \
    C:/OpenCV/opencv-3.2.0-install-with-cuda/include \

# OpenCV Libs
win32 {
OPENCV_LIBS_PATH = C:\OpenCV\opencv-3.2.0-install-with-cuda\x64\vc14\lib

    CONFIG(debug, debug|release) {
    LIBS += -L$$OPENCV_LIBS_PATH \
                -lopencv_world320d
    }

    CONFIG(release, debug|release) {
    LIBS += -L$$OPENCV_LIBS_PATH \
                -lopencv_world320
    }
}

# YOLO Libs
win32: LIBS += -L$$PWD/libs/ -lyolo_cpp_dll

INCLUDEPATH += $$PWD/libs
DEPENDPATH += $$PWD/libs

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/libs/yolo_cpp_dll.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/libs/libyolo_cpp_dll.a
