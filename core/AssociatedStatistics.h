#ifndef ASSOCIATEDSTATISTICS_H
#define ASSOCIATEDSTATISTICS_H

#include "core/DataSignal.h"
#include "core/Macroses.h"

// Неизменяемый класс относительных статистических характеристик с индексаций [ номер сравниваемого сигнала, номер базового окна, номер сравниваемого окна ]
class AssociatedStatistics {
public:
    AssociatedStatistics(QVector<DataSignal> const& vecDataSignal, int widthWindow, int shiftMainWindow, int shiftCompareWindow, int indMainSignal = 0);
    ~AssociatedStatistics() = default;
    AssociatedStatistics(AssociatedStatistics const&) = delete;
    // Интерфейс пользователя
    int size() const { return nSize_; }      // Число пар сигналов
    bool isEmpty() const { return !size(); } // Проверка на пустоту
private:
    // Оценочные методы
    void calcNumberOfWindows(); // Расчет числа окон по всем сигналам
    void findCorrespondence(); // Установление соответствия статистик и сигналов
    // Выделение памяти для полей структуры
    template<typename T>
    void allocateField(T& field); // Выделение памяти для поля типа ArrayStatCharacters и ArrayRegressionParams
    void allocateAllFields(); // Выделение памяти для всех полей
private:
    QVector<DataSignal> const& vecDataSignal_; // Ссылка на вектор сигналов
    int widthWindow_;                          // Ширина окна
    int shiftMainWindow_;                      // Смещение главного окна
    int shiftCompareWindow_;                   // Смещение сравниваемого окна
    int indMainSignal_ = 0;                    // Индекс главного сигнала
    // Параметры окон
    int nSize_ = 0;                            // Рабочий размер статистик
    int nSignal_ = 0;                          // Число сигналов
    QVector<int> vecNumberOfWindows_;          // Число окон по всем сигналам
    // Матрицы статистик
    QVector<int> vecIndCorrespond_;             // Вектор соответствия индекса в статистиках номеру сигнала
    ArrayRegressionParams regressionParams_;   // Параметры линейной регрессии
    ArrayStatCharacters distanceScatter_;      // Дистанция рассеяния
    ArrayStatCharacters similarityCoeffs_;     // Коэффициенты подобия сигналов
    ArrayStatCharacters amplitudeScatter_;     // Амплитуда рассеяния
    ArrayStatCharacters noiseCoeffs_;          // Коэффициенты шума
};

#endif // ASSOCIATEDSTATISTICS_H
