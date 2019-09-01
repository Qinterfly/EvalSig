#ifndef DIVISIONDATASIGNAL_H
#define DIVISIONDATASIGNAL_H

#include "core/DataSignal.h"
#include "core/PartsObject.h"
#include "core/NumericalFunctions.h"

class DivisionDataSignal{
public:
    DivisionDataSignal(DataSignal const& accel, DataSignal const& displacement, double levelStep, double overlapFactor,
        double smoothApproxFactor, double truncatePercent, double depthGluing, int lEstimationBound = 1, int rEstimationBound = -1);
    ~DivisionDataSignal() = default; // Деструктор
    // Расчетные методы
    void calculateLevels(); // Управляющий расчетный метод
    void calculatePowerSpectralDensity(WindowFunction windowFun, double overlapFactorWindow, int lengthSpectrum, int windowSmoothWidth); // Расчет плотности спектральной мощности
    // Установочные методы
    void setCalculationInd(int lEstimationBound, int rEstimationBound); // Задание расчетных границ
    void setLevelStep(double levelStep);                                // Задание величины смещения уровней
    void setOverlapFactor(double overlapFactor);                        // Задание величины перекрытия уровней
    void setSmoothApproxFactor(double smoothApproxFactor);              // Задание величины сглаживания перемещений
    void setTruncatePercent (double truncatePercent);                   // Задание процента усечения коротких фрагментов
    void setDepthGluing(double depthGluing);                            // Задание процента глубины склейки правой границы
    // Справочные методы
    int numberOfLevels() const { return nLevels_; } // Получить число уровней
    bool isEmpty() const { return nLevels_ == 0; }  // Проверка на пустоту
    DataSignal getDisplacement() const { return displacement_; } // Получить перемещения
    DataSignal getApproxDisplacement() const { return approxDisplacement_; } // Получить аппроксимированные перемещения
    // Файловые методы
    int writeAll(QString const& dirName) const;                                      // Сохранение всех данных
    int writeDisplacement(QString const& path, QString const& fileName) const;       // Сохранение перемещений
    int writeApproxDisplacement(QString const& path, QString const& fileName) const; // Сохранение аппроксимированных перемещений
    int writeSpectrum(QString const& dirName) const;                                 // Сохранение спектров склеек
    int writeGluedParts(QString const& dirName) const;                               // Сохранение склееных частей
    int writeInfo(QString const& dirName, QString const& fileName) const;            // Сохранение информации об уровнях
private:
    void createLevels(); // Создание расчетных уровней
    void assignLevels(PartsSignal & partsSignal, int firstLevelInd = 0, int lastLevelInd = -1); // Назначить уровни
    void truncateLevels(PartsObject & partsObject, int firstLevelInd = 0, int lastLevelInd = -1); // Усечение коротких фрагментов
    void derivativeLevels(PartsObject & partsObject, int firstLevelInd = 0, int lastLevelInd = -1); // Вычисление производных
    void glueLevels(QPair<PartsObject const&, QVector<DataSignal> &> const& linkageObjects, int firstLevelInd = 0, int lastLevelInd = -1); // Склейка по уровням
    void constructMonotoneLevels(QVector<PartsMonotone*> & vecPartsMonotone, int firstLevelInd = 0, int lastLevelInd = -1); // Выделение монотонных уровней
    template <typename T>
    void callMultiThread(T & someObject, void (DivisionDataSignal::*method)(T &, int, int)); // Вызов метода в многопоточном режиме
private:
    double levelStep_; // Величина смещения уровней
    double overlapFactor_; // Величина перекрытия уровней
    double smoothApproxFactor_; // Величина сглаживания перемещений
    double truncatePercent_; // Процент усечения коротких фрагментов
    double depthGluing_; // Процент глубины склейки правой границы
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
    // Монотонные
    PartsMonotone partsAccelIncrease; // Возрастающие ускорения
    PartsMonotone partsAccelNeutral;  // Нейтральные ускорения
    PartsMonotone partsAccelDecrease; // Убывающие ускорения
    QVector<PartsMonotone*> vecPartsAccelMonotone; // Контейнер для частей монотонных ускорений
    // Скленные
    QVector<DataSignal> gluedAccel_;         // Ускорения
    QVector<DataSignal> gluedAccelIncrease_; // Возрастающие ускорения
    QVector<DataSignal> gluedAccelNeutral_;  // Нейтральные ускорения
    QVector<DataSignal> gluedAccelDecrease_; // Убывающие ускорения
    // Спектры скленных частей
    QVector<DataSignal> spectrumAccel_;         // Ускорений
    QVector<DataSignal> spectrumAccelIncrease_; // Возрастающих ускорений
    QVector<DataSignal> spectrumAccelNeutral_;  // Нейтральных ускорений
    QVector<DataSignal> spectrumAccelDecrease_; // Убывающих ускоренияй
};

#endif // DIVISIONDATASIGNAL_H
