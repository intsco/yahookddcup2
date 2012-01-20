#-------------------------------------------------
#
# Project created by QtCreator 2011-11-23T23:07:00
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = predict
CONFIG   += console
CONFIG   -= app_bundle

QMAKE_CXXFLAGS += -fopenmp
//QMAKE_CXXFLAGS_DEBUG += -pg
//QMAKE_LFLAGS_DEBUG += -pg
LIBS += -lgomp

TEMPLATE = app


SOURCES += main.cpp \
    content_pred.cpp \
    optimize.cpp \
    estimate.cpp \
    itemnn_pred.cpp

HEADERS += \
    content_pred.h \
    core.h \
    main.h \
    optimize.h \
    estimate.h \
    itemnn_pred.h
