#ifndef STATISTICS_H
#define STATISTICS_H

#include "Macroses.h"
#include "DataSignal.h"
#include <QtMath>

struct Statistics{
    Statistics(QVector<DataSignal> const& vecData, int widthTimeWindow, double overlapFactor);
    ~Statistics() = default;
    Statistics(Statistics const&) = delete; // Запрет на копирование
    Statistics& operator=(Statistics const&) = delete; // и присваивание
private:
    template<typename T>
    void initAllocateField(T&); //Выделение памяти для полей структуры типа ArrayStatCharacters и ArrayRegressionParams
    int getMinSize(QVector<DataSignal> const&); // Получение минимальной длины сигнала
    void computeParams(QVector<DataSignal> const&); // Расчет характеристик
private:
    ArrayRegressionParams regressionParams_; // Параметры линейной регрессии
    ArrayStatCharacters distanceScatter_; // Дистанция рассеяния
    ArrayStatCharacters similarityCoeffs_; // Коэффициенты подобия сигналов
    ArrayStatCharacters amplitudeScatter_; // Амплитуда рассеяния
    int nSize_ = 0; // Длина вектора сигналов
    int minSizeSignals_ = 0; // Минимальная длина сигнала из группы
    int widthTimeWindow_; // Ширина временного окна
    double overlapFactor_; // Коэффициент перекрытия окон
    int nWindows_; // Число окон
};

#endif // STATISTICS_H
