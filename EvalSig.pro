#-------------------------------------------------
#
# Project created by QtCreator 2019-01-24T10:46:06
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = EvalSig
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS QCUSTOMPLOT_USE_LIBRARY

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++14
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

SOURCES += \
        core/DataSignal.cpp \
        core/Statistics.cpp \
        gui/MainWindow.cpp \
        gui/SignalProcessing.cpp \
        gui/Main.cpp \
        test/Tests.cpp \
        gui/InitializeMethods.cpp \
        gui/SetMethods.cpp \
        gui/ClearMethods.cpp \
        gui/ColorMapPlotting.cpp \
        gui/GraphPlotting.cpp \
        core/FileOperate.cpp \
        core/TimeWindowProperty.cpp \
        gui/UpdateMethods.cpp \
        core/NumericalFunctions.cpp \
        gui/SignalCharacteristicsWindow.cpp \
        gui/DataTransfer.cpp \
    include/csaps.cpp

HEADERS += \
        core/DataSignal.h \
        core/Statistics.h \
        core/Macroses.h \
        gui/MainWindow.h \
        test/Tests.h \
        core/FileOperate.h \
        core/PropertyDataSignal.h \
        core/TimeWindowProperty.h \
        core/NumericalFunctions.h \
        gui/SignalCharacteristicsWindow.h \
        include/csaps.h \
    include/fftw3.h

FORMS += \
        gui/MainWindow.ui \
    gui/SignalCharacteristicsWindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# Иконка к проекту
win32:RC_ICONS += $$PWD/gui/icons/app-icon.ico

RESOURCES += \
    gui/resource.qrc

# Include
INCLUDEPATH += $$PWD/include
DEPENDPATH += $$PWD/include

# Библиотека QCustomPlot
    # Windows
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/lib/ -lqcustomplot2
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/lib/ -lqcustomplotd2
    # Unix
unix:CONFIG(release, debug|release): LIBS += -L$$PWD/lib/ -lqcustomplot
else:unix:CONFIG(debug, debug|release): LIBS += -L$$PWD/lib/ -lqcustomplotd

# Библиотека FFTW
unix: LIBS += -L$$PWD/lib/ -lfftw3
else:win32: LIBS += "$$PWD/lib/libfftw3-3.dll"

# Библиотека QXlsx
QXLSX_PARENTPATH = $$PWD/include/QXlsx         # Директория проекта
QXLSX_HEADERPATH = $$QXLSX_PARENTPATH/header/ # Заголовочные файлы
QXLSX_SOURCEPATH = $$QXLSX_PARENTPATH/source/ # Файлы исходных кодов
include($$QXLSX_PARENTPATH/QXlsx.pri)       # Проект
