#ifndef SIGNALPROCESSING_H
#define SIGNALPROCESSING_H

#include "core/DataSignal.h"

// Функции обработки временных сигналов

DataSignal approximateSmoothSpline(DataSignal const& dataSignal, double smoothFactor, int nPoint = -1); // Аппроксимация сплайнами
DataSignal approximateLeastSquares(DataSignal const& dataSignal, int order, int nPoint = -1); // Аппроксимация по методу наименьших квадратов
QVector<DataSignal> integrate(DataSignal const& dataSignal, int orderIntegral, double smoothFactor); // Интегрирование
DataSignal interpolateLinear(DataSignal const& dataSignal, int nPoint); // Линейная интерполяция сигнала
DataSignal computePowerSpectralDensity(DataSignal const& dataSignal, QString const& typeWindow, int widthWindow, double overlapFactor); // Вычисление спектральной мощности сигнала
DataSignal bandpassFilter(DataSignal const& dataSignal, QString const& typeWindow, int widthWindow, double overlapFactor, QPair<double, double> const& freqSegment); // Фильтрация сигнала по частотам
DataSignal correct(DataSignal const& dataSignal, double smoothFactor); // Корректировка временного сигнала

// Вспомогательные
int previousPow2(int number); // Ближайшая предыдущая степень двойки

#endif // SIGNALPROCESSING_H
