#-------------------------------------------------
#
# Project created by QtCreator 2017-08-27T09:55:21
#
#-------------------------------------------------

QT       += core gui widgets opengl serialport network

CONFIG += tcp

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Lupa300Viewer
TEMPLATE = app

INCLUDEPATH += $$PWD/Filters $$PWD/Sources $$PWD/Sinks $$PWD/Support $$PWD/TCP

SOURCES += main.cpp\
    Filters/lauabstractfilter.cpp \
    Sinks/lau3dvideoglwidget.cpp \
    Sinks/lau3dvideoplayerwidget.cpp \
    Sources/lau3dcamera.cpp \
    Support/lauglwidget.cpp \
    Support/laulookuptable.cpp \
    Support/lauscan.cpp \
    TCP/lau3dvideotcpclient.cpp \
    TCP/lau3dvideotcpglfilter.cpp \
    TCP/lau3dvideotcpwidget.cpp \
    Support/laumemoryobject.cpp \
    TCP/lau3dvideotcpserver.cpp \
    Support/lauvideoplayerlabel.cpp \
    Sinks/lau3dscanglwidget.cpp \
    Support/lauscaninspector.cpp \
    Sinks/lau3dfiducialglwidget.cpp \

HEADERS  += TCP/lau3dvideotcpclient.h \
    TCP/lau3dvideotcpglfilter.h \
    TCP/lau3dvideotcpwidget.h \
    Filters/lauabstractfilter.h \
    Sinks/lau3dvideoglwidget.h \
    Sinks/lau3dvideoplayerwidget.h \
    Sources/lau3dcamera.h \
    Support/lauglwidget.h \
    Support/laulookuptable.h \
    Support/lauscan.h \
    TCP/lau3dvideotcpclient.h \
    TCP/lau3dvideotcpglfilter.h \
    TCP/lau3dvideotcpwidget.h \
    Support/laumemoryobject.h \
    TCP/lau3dvideotcpserver.h \
    Support/lauvideoplayerlabel.h \
    Sinks/lau3dscanglwidget.h \
    Support/lauscaninspector.h \
    Sinks/lau3dfiducialglwidget.h

unix:macx {
    QMAKE_CXXFLAGS += -msse2 -msse3 -mssse3 -msse4.1
    INCLUDEPATH    += /usr/local/include /usr/local/include/eigen3
    DEPENDPATH     += /usr/local/include /usr/local/include/eigen3
    LIBS           += /usr/local/lib/libtiff.5.dylib

    tcp {
        INCLUDEPATH   += /usr/local/include
        DEPENDPATH    += /usr/local/include
        LIBS          += /usr/local/lib/libQtZeroConf.dylib
    }
}

unix:!macx {
    CONFIG += c++11
    QMAKE_CXXFLAGS += -msse2 -msse3 -mssse3 -msse4.1
    INCLUDEPATH    += /usr/include /usr/include/eigen3
    DEPENDPATH     += /usr/include /usr/include/eigen3
    LIBS           += -ltiff

    tcp {
        LIBS          += -lQtZeroConf
    }
}

win32 {
    INCLUDEPATH += $$quote(C:/usr/include)
    DEPENDPATH  += $$quote(C:/usr/include)
    LIBS        += -L$$quote(C:/usr/lib) -llibtiff_i -lopengl32

    tcp {
        CONFIG(release, debug|release): LIBS += -lQtZeroConf
        CONFIG(debug, debug|release):   LIBS += -lQtZeroConfd
    }
}

RESOURCES += \
    Shaders/shaders.qrc \
    Images/lauvideoplayerlabel.qrc
