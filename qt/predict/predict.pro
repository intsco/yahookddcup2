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
LIBS += -lgomp

TEMPLATE = app


SOURCES += main.cpp \
    content_pred.cpp \
    optimize.cpp \
    estimate.cpp \
    itemnn_pred.cpp \
    check_i2i_weights.cpp \
    binsvd_pred.cpp \
    loader.cpp \
    blend_pred.cpp

HEADERS += \
    content_pred.h \
    core.h \
    optimize.h \
    estimate.h \
    itemnn_pred.h \
    check_i2i_weights.h \
    binsvd_pred.h \
    loader.h \
    blend_pred.h
