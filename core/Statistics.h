#ifndef STATISTICS_H
#define STATISTICS_H

#include "Macroses.h"
#include "DataSignal.h"
#include "TimeWindowProperty.h"

// Класс статистических характеристик
struct Statistics{
    Statistics(QVector<DataSignal> & vecDataSignal, int widthTimeWindow, int shiftTimeWindow, int leftEstimationBoundary, int rightEstimationBoundary);
    ~Statistics() = default; // Деструктор
    Statistics(Statistics const&) = delete; // Запрет на копирование
    Statistics& operator=(Statistics const&) = delete; // Присваивание осуществяется в случае полного пересчета
    // Интерфейс пользователя
    int size() const { return nSize_; } // Текущий размер матрицы статистик
    bool isEmpty() const { return !size(); } // Проверка на пустоту
    int minSizeSignals() const { return minSizeSignals_; } // Минимальная длина сигнала из группы
    int getNumberOfWindows() const { return windowProperty.nWindows_; } // Получить число временных окон (без учета среднего)
    QPair<int, int> const& getEstimationBoundaries() const { return estimationBoundaries_; } // Получение границ расчета
        // Статистические характеристики
    ArrayRegressionParams const& getRegressionParams() const { return regressionParams_; } // Получение регрессионных параметров
    ArrayStatCharacters const& getDistanceScatter() const { return distanceScatter_; }     // Получение дистанций рассеяния
    ArrayStatCharacters const& getSimilarityCoeffs() const { return similarityCoeffs_; }   // Получение коэффициентов подобия сигналов
    ArrayStatCharacters const& getAmplitudeScatter() const { return amplitudeScatter_; }   // Получение амплитуд рассеяния
    ArrayStatCharacters const& getNoiseCoeffs() const { return noiseCoeffs_; }             // Получение коэффициентов шума
        // Метрики
    double getMeanSegment(int ind) const { return meanSegment_[ind]; }              // Среднее на отрезке
    double getSquareMeanSegment(int ind) const { return squareMeanSegment_[ind]; }  // Среднее квадратическое отклонение на отрезке
    double getMinSegment(int ind) const { return minMaxSegment_[ind].first; }       // Минимум на отрезке
    double getMaxSegment(int ind) const { return minMaxSegment_[ind].second; }      // Максимум на отрезке
    double getLocalDeviationSegment(int ind) const { return localDeviationSegment_[ind]; } // Локальное отклонение
    bool addSignal(DataSignal const& dataSignal); // Добавление сигнала
    bool removeSignal(int deleteInd); // Удаление сигнала
    void setWindowProperty(int widthTimeWindow, int shiftTimeWindow); // Изменение свойств окна
    void setEstimationBoundaries(int leftBound, int rightBound); // Выставление расчетных границ
    int writeAllStatistics(QString const& dirName) const; // Сохранение всех статистик
    int writeSignalList(QString const& path, QString const& fileName) const; // Запись списка сигналов
    int writeAllMetrics(QString const& dirName, QString const& fileName) const; // Сохранение метрик по всем сигналам
private:
    // Выделение памяти для полей структуры типа ArrayStatCharacters и ArrayRegressionParams
    template<typename T>
    void allocateField(T& field, int beginColInd, int fullSize); // При расширении объекта
    template<typename T>
    void removeField(T& field, int deleteInd);                   // При сжатии объекта
    // Методы-обертки для выделения памяти для всех полей
    void allocateAllFields(int beginColInd, int fullSize); // При расширении для всех полей
    void removeAllFields(int deleteInd);                   // При сжатии для всех полей
    // Методы проверки
    void checkEstimationBoundaries(); // Проверка корректности расчетных границ
    // Расчет статистических характеристик
    int calcMinSizeSignals(); // Получение минимальной длины сигнала
    void fullCompute(); // Полный расчет характеристик
    void partialCompute(); // Частичный расчет характеристик
        // Тело цикла пересчета для дистанций, амплитуд и регрессионных параметров
    void calcDistanceAmplitudeRegression(int i, int j);
        // Тело цикла для расчета коэффициентов подобия
    void calcSimilarity(int i, int j);
    // Сохранение выбранной статистики
    template<typename T>
    int writeStatistic(T const& stat, QString const& dirName, QString const& statName) const; // ArrayRegressionParams и ArrayStatCharacters
    int writeMeanStatistics(QString const& dirName, QString const& fileName) const; // Сохранение средних значений статистик
    // Вспомогательные функции получения оконного распределения статистики
    QVector<double> getWindowStatisticData(ArrayRegressionParams const& stat, int i, int j) const; // ArrayRegressionParams
    QVector<double> getWindowStatisticData(ArrayStatCharacters const& stat, int i, int j) const; // ArrayStatCharacters
    // Расчет метрик сигналов
    void calcAllMetrics(); // Расчет всех метрик
    void calcMetric(int iSignal); // Расчет метрики сигнала по индексу
    // Методы-обертки для выделения памяти для всех метрик
    void allocateAllMetrics(); // При расширении для всех метрик
    void removeAllMetrics(int deleteInd);   // При сжатии для всех метрик
private:
    QVector<DataSignal> * const pVecDataSignal = nullptr; // Указатель на вектор сигналов
    ArrayRegressionParams regressionParams_; // Параметры линейной регрессии
    ArrayStatCharacters distanceScatter_; // Дистанция рассеяния
    ArrayStatCharacters similarityCoeffs_; // Коэффициенты подобия сигналов
    ArrayStatCharacters amplitudeScatter_; // Амплитуда рассеяния
    ArrayStatCharacters noiseCoeffs_; // Коэффициенты шума
    int nSize_ = 0; // Размер матрицы статистических параметров
    int minSizeSignals_ = 0; // Минимальная длина сигнала из группы
    QPair <int, int> estimationBoundaries_; // Границы расчета
    TimeWindowProperty windowProperty; // Свойства окна выделения характеристик
    // Метрики сигналов на интервале
    QVector<double> meanSegment_; // Среднее значение
    QVector<double> squareMeanSegment_; // Среднее квадратическое отклонение
    QVector< QPair<double, double> > minMaxSegment_; // Минимум и максимум
    QVector<double> localDeviationSegment_; // Локальное отклонение
};

#endif // STATISTICS_H
