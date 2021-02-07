#ifndef TIMEWINDOWPROPERTY_H
#define TIMEWINDOWPROPERTY_H

#include <QString>
#include <QPair>
#include "FileOperate.h"

// Свойства окна выделения характеристик
// закрытое поле класса Statistics
struct TimeWindowProperty {
    TimeWindowProperty(int width, int shiftWindow, QPair<int, int> const& estimationBoundaries, int minSizeSignals);
    void calcWindowParams(QPair<int, int> const& estimationBoundaries, int minSizeSignals); // Расчет параметров окна
    int writeWindowParams(QString const& path, QString const& fileName) const; // Запись параметров окна в файл

    int width_;             // Ширина временного окна
    int nWindows_;          // Число окон
    int shiftWindow_;       // Шаг смещение левой границы окна по времени
};

#endif // TIMEWINDOWPROPERTY_H
