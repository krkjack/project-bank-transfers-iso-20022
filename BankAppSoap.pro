QT       += core gui sql
greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
include(SlidingStackedWidget/SlidingStackedWidget.pri)
CONFIG += c++11

QMAKE_CXXFLAGS += -Wno-deprecated
RC_ICONS = icon.ico
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    libiban.cpp \
    main.cpp \
    mainwindow.cpp \
    soapC.cpp \
    stdsoap2.cpp \
    tinyxml2.cpp \
    utils.cpp

HEADERS += \
    ../../Desktop/libiban-master/src/libiban.h \
    libiban.h \
    mainwindow.h \
    ns1.nsmap \
    soapH.h \
    soapStub.h \
    stdsoap2.h \
    tinyxml2.h \
    utils.h

FORMS += \
    mainwindow.ui


CONFIG += lrelease
CONFIG += embed_translations

INCLUDEPATH += "C://gsoap-2.8//gsoap//"
LIBS += "C://Qt//Tools//mingw900_64//x86_64-w64-mingw32//lib//libws2_32.a"
