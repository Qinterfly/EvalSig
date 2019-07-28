#ifndef DIVISIONDATASIGNAL_H
#define DIVISIONDATASIGNAL_H

#include <QSharedPointer>
#include "core/DataSignal.h"

struct DivisionDataSignal{
    DivisionDataSignal(DataSignal const& dataSignal, double levelStep, double overlapFactor, double smoothFactor,
                       int lEstimationBoundary = 1, int rEstimationBoundary = -1);
    void setCalculationInd(int lEstimationBoundary, int rEstimationBoundary); // Задание расчетных границ
    // Файловые методы
    int writeDisplacement(QString const& path, QString const& fileName); // Сохранение перемещений
private:
    void createLevels(); // Создание расчетных уровней

private:
    double levelStep_; // Величина смещения уровней
    double overlapFactor_; // Величина перекрытия уровней
    double smoothFactor_; // Величина сглаживания
    DataSignal const * const ptrAccel_; // Указатель на полный временной сигнал ускорений
    DataSignal displacement_; // Перемещение
    QPair <int, int> calculationInd_; // Индексы границ расчета
    QVector<double> lowBoundLevels_, upperBoundLevels_; // Нижние, верхние границы уровней
    QVector<int> indLevels_; // Индексы уровней
    int nLevels = 0;
};

#endif // DIVISIONDATASIGNAL_H
