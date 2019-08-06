#ifndef DIVISIONDATASIGNAL_H
#define DIVISIONDATASIGNAL_H

#include "core/DataSignal.h"
#include "PartsObject.h"

class DivisionDataSignal{
public:
    DivisionDataSignal(DataSignal const& dataSignal, double levelStep, double overlapFactor, double smoothIntegFactor,
                       double smoothApproxFactor, double truncatePercent, int lEstimationBound= 1, int rEstimationBound = -1);
    ~DivisionDataSignal() = default; // Деструктор
    void setCalculationInd(int lEstimationBound, int rEstimationBound); // Задание расчетных границ
    int numberOfLevels() const { return nLevels_; } // Получить число уровней
    DataSignal getDisplacement() const { return displacement_; } // Получить перемещения
    DataSignal getApproxDisplacement() const { return approxDisplacement_; } // Получить аппроксимированные перемещения
    // Файловые методы
    int writeDisplacement(QString const& path, QString const& fileName); // Сохранение перемещений
    int writeApproxDisplacement(QString const& path, QString const& fileName); // Сохранение аппроксимированных перемещений
private:
    void calculate(); // Управляющий расчетный метод
    void createLevels(); // Создание расчетных уровней
    void assignLevels(PartsSignal & partsSignal, int firstLevelInd = 0, int lastLevelInd = -1); // Назначить уровни
    void truncateLevels(PartsObject & partsObject, int firstLevelInd = 0, int lastLevelInd = -1); // Усечение коротких фрагментов
    void derivativeLevels(PartsObject & partsObject, int firstLevelInd = 0, int lastLevelInd = -1); // Вычисление производных
    void glueLevels(PartsObject & partsObject, int firstLevelInd = 0, int lastLevelInd = -1); // Склейка по уровням
    void constructMonotoneLevels(QVector<PartsMonotone*> & vecPartsMonotone, int firstLevelInd = 0, int lastLevelInd = -1); // Выделение монотонных уровней
    template <typename T>
    void callMultiThread(T & someObject, void (DivisionDataSignal::*method)(T &, int, int)); // Вызов метода в многопоточном режиме
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
    // Части сигналов
    PartsSignal partsAccel;        // Ускорения
    PartsSignal partsDisplacement; // Перемещения
    // Скленные
    PartsSignal partsAccelGlued;   // Ускорения
    // Монотонные
    PartsMonotone partsAccelIncrease; // Возрастающие ускорения
    PartsMonotone partsAccelNeutral;  // Нейтральные ускорения
    PartsMonotone partsAccelDecrease; // Убывающие ускорения
    QVector<PartsMonotone*> vecPartsAccelMonotone; // Контейнер для частей монотонных ускорений
};

#endif // DIVISIONDATASIGNAL_H
