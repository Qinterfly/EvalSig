#ifndef TIMEWINDOWPROPERTY_H
#define TIMEWINDOWPROPERTY_H

#include <QString>
#include "FileOperate.h"

// Свойства окна выделения характеристик
// закрытое поле класса Statistics
struct TimeWindowProperty {
    TimeWindowProperty(int width, int shiftWindow, int sizeSignals);
    void calcWindowParams(int); // Расчет параметров окна
    int writeWindowParams(QString const& path, QString const& fileName); // Запись параметров окна

    int width_;             // Ширина временного окна
    int nWindows_;          // Число окон
    int shiftWindow_;       // Шаг смещение левой границы окна по времени
};

#endif // TIMEWINDOWPROPERTY_H
