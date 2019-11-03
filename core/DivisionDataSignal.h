#ifndef DIVISIONDATASIGNAL_H
#define DIVISIONDATASIGNAL_H

#include "core/DataSignal.h"
#include "core/PartsObject.h"
#include "core/NumericalFunctions.h"

// Разбиение базового временного сигнала по уровням согласно опорному
class DivisionDataSignal {
public:
    DivisionDataSignal(DataSignal const& base, DataSignal const& support, DataSignal const& approxSupport,
                       double levelStep, double overlapFactor, double truncatePercent, double depthGluing,
                       int lEstimationBound = 1, int rEstimationBound = -1);
    ~DivisionDataSignal() = default; // Деструктор
    // Расчетные методы
    static void createLevels(DataSignal const& approxSupport, QPair <int, int> const& calculationInd, double overlapFactor, double levelStep,
                             QVector<double> & lowBoundLevels, QVector<double> & upperBoundLevels, QVector<int> & indLevels, int & nLevels); // Создание расчетных уровней
    void calculateLevels(); // Управляющий расчетный метод
    void calculatePowerSpectralDensity(WindowFunction windowFun, double overlapFactorWindow, int lengthSpectrum, int windowSmoothWidth); // Расчет плотности спектральной мощности
    // Установочные методы
    void setCalculationInd(int lEstimationBound, int rEstimationBound); // Задание расчетных границ
    void setLevelStep(double levelStep);                                // Задание величины смещения уровней
    void setOverlapFactor(double overlapFactor);                        // Задание величины перекрытия уровней
    void setTruncatePercent (double truncatePercent);                   // Задание процента усечения коротких фрагментов
    void setDepthGluing(double depthGluing);                            // Задание процента глубины склейки правой границы
    // Справочные методы
    int numberOfLevels() const { return nLevels_; } // Получить число уровней
    bool isEmpty() const { return nLevels_ == 0; }  // Проверка на пустоту
    DataSignal getSupport() const { return support_; } // Получить опорный сигнал
    DataSignal getApproxSupport() const { return approxSupport_; } // Получить аппроксимированный опорный сигнал
    // Файловые методы
    int writeAll(QString const& dirName) const;                                  // Сохранение всех данных
    int writeSupport(QString const& path, QString const& fileName) const;        // Сохранение опорного сигнала
    int writeApproxSupport(QString const& path, QString const& fileName) const;  // Сохранение аппроксимации опорного сигнала
    int writeSpectrum(QString const& dirName) const;                             // Сохранение спектров склеек
    int writeGluedParts(QString const& dirName) const;                           // Сохранение склееных частей
    int writeInfo(QString const& dirName, QString const& fileName) const;        // Сохранение информации об уровнях
private:
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
    double truncatePercent_; // Процент усечения коротких фрагментов
    double depthGluing_; // Процент глубины склейки правой границы
    DataSignal base_;    // Базовый сигнал
    DataSignal support_; // Опорный сигнал
    DataSignal approxSupport_; // Аппроксимированный опорный сигнал
    QPair <int, int> calculationInd_; // Индексы границ расчета
    QVector<double> lowBoundLevels_, upperBoundLevels_; // Нижние, верхние границы уровней
    QVector<int> indLevels_; // Индексы уровней
    int nLevels_ = 0; // Число уровней
    // Части сигналов
    PartsSignal partsBase;    // Базовые
    PartsSignal partsSupport; // Опорные
    // Монотонные
    PartsMonotone partsBaseIncrease; // Возрастающие базовые
    PartsMonotone partsBaseNeutral;  // Нейтральные базовые
    PartsMonotone partsBaseDecrease; // Убывающие базовые
    QVector<PartsMonotone*> vecPartsBaseMonotone; // Контейнер для частей монотонных базовых
    // Скленные
    QVector<DataSignal> gluedBase_;         // Ускорения
    QVector<DataSignal> gluedBaseIncrease_; // Возрастающие базовые
    QVector<DataSignal> gluedBaseNeutral_;  // Нейтральные базовые
    QVector<DataSignal> gluedBaseDecrease_; // Убывающие базовые
    // Спектры скленных частей
    QVector<DataSignal> spectrumBase_;         // Базовых
    QVector<DataSignal> spectrumBaseIncrease_; // Возрастающих базовых
    QVector<DataSignal> spectrumBaseNeutral_;  // Нейтральных базовых
    QVector<DataSignal> spectrumBaseDecrease_; // Убывающих базовых
};

#endif // DIVISIONDATASIGNAL_H
