#ifndef SIGNALPROCESSING_H
#define SIGNALPROCESSING_H

#include "core/DataSignal.h"

// Функции обработки временных сигналов

DataSignal approximateSmoothSpline(DataSignal const& dataSignal, double smoothFactor); // Аппроксимация сплайнами
QVector<DataSignal> integrate(DataSignal const& dataSignal, int orderIntegral, double smoothFactor); // Интегрирование
DataSignal computePowerSpectralDensity(DataSignal const& dataSignal, QString const& typeWindow, int widthWindow, double overlapFactor, double smoothFactor); // Вычисление спектральной мощности сигнала


#endif // SIGNALPROCESSING_H
