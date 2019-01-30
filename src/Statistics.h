#ifndef STATISTICS_H
#define STATISTICS_H

#include "Macroses.h"
#include "DataSignal.h"
#include <QtMath>

// Свойства окна выделения характеристик
struct TimeWindowProperty {
    TimeWindowProperty(int width, double overlapFactor, int sizeSignals);
    int width_;             // Ширина временного окна
    double overlapFactor_;  // Коэффициент перекрытия окон
    int nWindows_;          // Число окон
    int shiftWindow_;       // Шаг смещение левой границы окна по времени
};

// Класс статистических характеристик
struct Statistics{
    Statistics(QVector<DataSignal> & vecDataSignal, int widthTimeWindow, double overlapFactor);
    ~Statistics(){}
    Statistics(Statistics const&) = delete; // Запрет на копирование
    Statistics& operator=(Statistics const&) = delete; // Присваивание осуществяется в случае полного пересчета
    // Интерфейс пользователя
    int size() const; // Текущий размер матрицы статистик
    bool isEmpty() const; // Проверка на пустоту
    int minSizeSignals() const; // Минимальная длина сигнала из группы
    bool addSignal(DataSignal const& dataSignal); // Добавление сигнала
    bool removeSignal(int deleteInd); // Удаление сигнала
private:
    // Выделение памяти для полей структуры типа ArrayStatCharacters и ArrayRegressionParams
    template<typename T>
    void allocateField(T& field, int beginColInd, int fullSize); // При расширении объекта
    template<typename T>
    void removeField(T& field, int deleteInd);                   // При сжатии объекта
    // Методы-обертки для выделения памяти для всех полей
    void allocateAllFields(int beginColInd, int fullSize); // При расширении для всех полей
    void removeAllFields(int deleteInd);                   // При сжатии для всех полей
    // Расчет статистических характеристик
    int calcMinSizeSignals(); // Получение минимальной длины сигнала
    void fullCompute(); // Полный расчет характеристик
    void partialCompute(); // Частичный расчет характеристик
        // Тело цикла пересчета для дистанций, амплитуд и регрессионных параметров
    void calcDistanceAmplitudeRegression(int shiftWindow, int i, int j);
        // Тело цикла для расчета коэффициентов подобия
    void calcSimilarity(int shiftWindow, int i, int j);
private:
    QVector<DataSignal> * const pVecDataSignal = nullptr; // Указатель на вектор сигналов
    ArrayRegressionParams regressionParams_; // Параметры линейной регрессии
    ArrayStatCharacters distanceScatter_; // Дистанция рассеяния
    ArrayStatCharacters similarityCoeffs_; // Коэффициенты подобия сигналов
    ArrayStatCharacters amplitudeScatter_; // Амплитуда рассеяния
    int nSize_ = 0; // Размер матрицы статистических параметров
    int minSizeSignals_ = 0; // Минимальная длина сигнала из группы
    TimeWindowProperty windowProperty; // Свойства окна выделения характеристик
};

#endif // STATISTICS_H
