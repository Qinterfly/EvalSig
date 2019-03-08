#ifndef SIGNALPROCESSING_H
#define SIGNALPROCESSING_H

#include "core/DataSignal.h"

// Функции обработки временных сигналов

DataSignal approximateSmoothSpline(DataSignal const& dataSignal, double smoothFactor); // Аппроксимация сплайнами
QVector<DataSignal> integrate(DataSignal const& dataSignal, int orderIntegral, double smoothFactor); // Интегрирование

#endif // SIGNALPROCESSING_H
