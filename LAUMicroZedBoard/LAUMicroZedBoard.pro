QT += core network

TARGET = LAUMicroZedBoard

TEMPLATE = app

HEADERS += lau3dcamera.h \
           laumemoryobject.h \
           lau3dvideotcpserver.h \
           laulupa300camera.h

SOURCES += main.cpp \
           lau3dcamera.cpp \
           laumemoryobject.cpp \
           lau3dvideotcpserver.cpp \
           laulupa300camera.cpp

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

unix:macx{
    INCLUDEPATH  += /usr/local/include
    DEPENDPATH   += /usr/local/include
    LIBS         += -L/usr/local/lib/ -lQtZeroConf /usr/local/lib/libtiff.5.dylib
}

unix:!macx {
    CONFIG += console
    CONFIG -= app_bundle
    CONFIG += c++11
    INCLUDEPATH    += /usr/local/include /usr/local/include/eigen3
    INCLUDEPATH    += /usr/include
    DEPENDPATH     += /usr/include
    DEPENDPATH     += /usr/local/include /usr/local/include/eigen3
    #LIBS           += /usr/local/lib/libtiff.5.dylib
    #LIBS           += /usr/local/lib/libQtZeroConf.dylib
    LIBS           += -L/usr/lib -ltiff
    LIBS           += /root/Engineering/tcpip/QtZeroConf-master/libQtZeroConf.so
    LIBS           += /root/Engineering/tcpip/QtZeroConf-master/libQtZeroConf.so.1
    LIBS           += /root/Engineering/tcpip/QtZeroConf-master/libQtZeroConf.so.1.0
    #LIBS           += /root/Engineering/tcpip/QtZeroConf-master/libQtZeroConf.so.1.0.0
    #LIBS           += /root/Engineering/tcpip/QtZeroConf-master/qzeroconf.h
    #LIBS           += /root/Engineering/tcpip/QtZeroConf-master/qzeroconf.pri
}

