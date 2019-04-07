#ifndef STATISTICS_H
#define STATISTICS_H

#include "Macroses.h"
#include "DataSignal.h"
#include "TimeWindowProperty.h"

// Класс статистических характеристик
struct Statistics{
    Statistics(QVector<DataSignal> & vecDataSignal, int widthTimeWindow, int shiftTimeWindow, int leftEstimationBoundary, int rightEstimationBoundary);
    ~Statistics(){}
    Statistics(Statistics const&) = delete; // Запрет на копирование
    Statistics& operator=(Statistics const&) = delete; // Присваивание осуществяется в случае полного пересчета
    // Интерфейс пользователя
    int size() const { return nSize_; } // Текущий размер матрицы статистик
    bool isEmpty() const { return !size(); } // Проверка на пустоту
    int minSizeSignals() const { return minSizeSignals_; } // Минимальная длина сигнала из группы
    int getNumberOfWindows() const { return windowProperty.nWindows_; } // Получить число временных окон (без учета среднего)
    QPair<int, int> const& getEstimationBoundaries() const { return estimationBoundaries_; } // Получение границ расчета
    ArrayRegressionParams const& getRegressionParams() const { return regressionParams_; } // Получение регрессионных параметров
    ArrayStatCharacters const& getDistanceScatter() const { return distanceScatter_; } // Получение дистанций рассеяния
    ArrayStatCharacters const& getSimilarityCoeffs() const { return similarityCoeffs_; } // Получение коэффициентов подобия сигналов
    ArrayStatCharacters const& getAmplitudeScatter() const { return amplitudeScatter_; } // Получение амплитуд рассеяния
    ArrayStatCharacters const& getNoiseCoeffs() const { return noiseCoeffs_; } // Получение коэффициентов шума
    bool addSignal(DataSignal const& dataSignal); // Добавление сигнала
    bool removeSignal(int deleteInd); // Удаление сигнала
    void setWindowProperty(int widthTimeWindow, int shiftTimeWindow); // Изменение свойств окна
    void setEstimationBoundaries(int leftBound, int rightBound); // Выставление расчетных границ
    int writeAllStatistics(QString const& dirName); // Сохранение всех статистик
    int writeSignalList(QString const& path, QString const& fileName); // Запись списка сигналов
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
    int writeStatistic(T const& stat, QString const& dirName, QString const& statName); // ArrayRegressionParams и ArrayStatCharacters
    // Вспомогательные функции получения оконного распределения статистики
    QVector<double> getWindowStatisticData(ArrayRegressionParams const& stat, int i, int j); // ArrayRegressionParams
    QVector<double> getWindowStatisticData(ArrayStatCharacters const& stat, int i, int j); // ArrayStatCharacters
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
};

#endif // STATISTICS_H
