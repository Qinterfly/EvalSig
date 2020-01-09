#-------------------------------------------------
#
# Project created by QtCreator 2019-01-24T10:46:06
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = EvalSig
TEMPLATE = app

DEFINES += QCUSTOMPLOT_USE_LIBRARY

QMAKE_CXXFLAGS += "-Wno-deprecated-copy"

CONFIG += c++17
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

SOURCES += \
        core/AssociatedStatistics.cpp \
        core/CalculationTemplate.cpp \
        core/DataSignal.cpp \
        core/DivisionDataSignal.cpp \
        core/PartsObject.cpp \
        core/Statistics.cpp \
        gui/AssociatedStatisticsWindow.cpp \
        gui/CalculationTemplateWindow.cpp \
    gui/DoubleProgressDialog.cpp \
        gui/LevelsWindow.cpp \
        gui/MainWindow.cpp \
        gui/QCPColorCurve.cpp \
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
        gui/QCustomPlotZoom.cpp \
     include/csaps.cpp


HEADERS += \
        core/AssociatedStatistics.h \
        core/CalculationTemplate.h \
        core/DataSignal.h \
        core/DivisionDataSignal.h \
        core/PartsObject.h \
        core/Statistics.h \
        core/Macroses.h \
        gui/AssociatedStatisticsWindow.h \
    gui/CalculationTemplateWindow.h \
    gui/DoubleProgressDialog.h \
        gui/LevelsWindow.h \
        gui/MainWindow.h \
        gui/QCPColorCurve.h \
        test/Tests.h \
        core/FileOperate.h \
        core/PropertyDataSignal.h \
        core/TimeWindowProperty.h \
        core/NumericalFunctions.h \
        gui/SignalCharacteristicsWindow.h \
        gui/QCustomPlotZoom.h \
        include/csaps.h \
    include/fftw3.h

FORMS += \
    gui/AssociatedStatisticsWindow.ui \
    gui/CalculationTemplateWindow.ui \
    gui/DoubleProgressDialog.ui \
    gui/LevelsWindow.ui \
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
QXLSX_PARENTPATH = $$PWD/include/QXlsx        # Директория проекта
QXLSX_HEADERPATH = $$QXLSX_PARENTPATH/header/ # Заголовочные файлы
QXLSX_SOURCEPATH = $$QXLSX_PARENTPATH/source/ # Файлы исходных кодов
include($$QXLSX_PARENTPATH/QXlsx.pri)         # Проект
