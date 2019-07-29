#ifndef DIVISIONDATASIGNAL_H
#define DIVISIONDATASIGNAL_H

#include <QSharedPointer>
#include "core/DataSignal.h"

using partsDouble = QVector< QVector<double> >;
using partsInt = QVector< QVector<int> >;

struct DivisionDataSignal{
    DivisionDataSignal(DataSignal const& dataSignal, double levelStep, double overlapFactor, double smoothIntegFactor,
                       double smoothApproxFactor, double truncatePercent, int lEstimationBound= 1, int rEstimationBound = -1);
    ~DivisionDataSignal() = default; // Деструктор
    void setCalculationInd(int lEstimationBoundary, int rEstimationBoundary); // Задание расчетных границ
    int numberOfLevels() const { return nLevels_; } // Получить число уровней
    DataSignal getDisplacement() const { return displacement_; } // Получить перемещения
    DataSignal getApproxDisplacement() const { return approxDisplacement_; } // Получить аппроксимированные перемещения
    // Файловые методы
    int writeDisplacement(QString const& path, QString const& fileName); // Сохранение перемещений
    int writeApproxDisplacement(QString const& path, QString const& fileName); // Сохранение аппроксимированных перемещений
private:
    void createLevels(); // Создание расчетных уровней
    void assignLevels(int firstLevelInd = 0, int lastLevelInd = -1); // Назначить уровни
    void truncateLevels(int firstLevelInd = 0, int lastLevelInd = -1); // Усечение коротких фрагментов
    void derivativeLevels(int firstLevelInd = 0, int lastLevelInd = -1); // Вычисление производных
    void callMultiThread(void (DivisionDataSignal::*method)(int, int)); // Вызов метода в многопоточном режиме
private:
    double levelStep_; // Величина смещения уровней
    double overlapFactor_; // Величина перекрытия уровней
    double smoothIntegFactor_; // Величина сглаживания при интегрировании
    double smoothApproxFactor_; // Величина сглаживания перемещений
    double truncatePercent_; // Процент усечения коротких фрагментов
    DataSignal accel_; // Указатель на полный временной сигнал ускорений
    DataSignal displacement_; // Перемещение
    DataSignal approxDisplacement_; // Аппроксимированные перемещения
    QPair <int, int> calculationInd_; // Индексы границ расчета
    QVector<double> lowBoundLevels_, upperBoundLevels_; // Нижние, верхние границы уровней
    QVector<int> indLevels_; // Индексы уровней
    int nLevels_ = 0; // Число уровней
    // Части перемещений
    partsInt parTime_; // Время
    partsDouble parAccel_; // Ускорения
    partsInt parFlags_; // Флаги концов фрагментов
    partsInt parInd_; // Индексы концов фрагментов
    QVector<int> lengthLevels_; // Длины уровней
    QVector<int> nFragmentLevels_; // Число фрагментов на уровнях
};

#endif // DIVISIONDATASIGNAL_H
