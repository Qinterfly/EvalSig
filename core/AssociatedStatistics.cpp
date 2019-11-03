#include "AssociatedStatistics.h"

// ---- Неизменяемый класс относительных статистических характеристик ------------------------------------------

AssociatedStatistics::AssociatedStatistics(QVector<DataSignal> const& vecDataSignal, int widthWindow, int shiftMainWindow,
                                           int shiftCompareWindow, int indMainSignal) : vecDataSignal_(vecDataSignal),
    widthWindow_(widthWindow), shiftMainWindow_(shiftMainWindow), shiftCompareWindow_(shiftCompareWindow), indMainSignal_(indMainSignal)
{
    nSignal_ = vecDataSignal.size(); // Число сигналов
    if (nSignal_) nSize_ = nSignal_ - 1; // Определение размера матрицы
    // Выделение памяти
    vecNumberOfWindows_.resize(nSignal_); // Число окон по сигналам
    vecIndCorrespond_.resize(nSize_);  // Вектор соответствия индекса в статистиках номеру сигнала
    // Предварительные оценки
    calcNumberOfWindows(); // Числа окон по сигналам
    allocateAllFields(); // Выделение памяти для всех полей
    findCorrespondence(); // Установление соответствия статистик и сигналов
}

// Расчет числа окон по всем сигналам
void AssociatedStatistics::calcNumberOfWindows(){
    int shiftWindow = 0; // Параметры разбиения текущего сигнала
    for (int i = 0; i != nSignal_; ++i){
        shiftWindow = shiftCompareWindow_;
        if (i == indMainSignal_) shiftWindow = shiftMainWindow_; // Изменение параметров окна для главного сигнала
        int sizeSignal = vecDataSignal_[i].size();
        int currWindow = 0; // Номер текущего окна
        // Пока текущая левая граница не достигнет конца правой расчетной границы
        for (int s = 0; s < sizeSignal; ){
            if (s + widthWindow_ > sizeSignal && currWindow != 0) // Проверка правой границы
                break; // Разрешены только цельные окна
            s += shiftWindow; // Сдвиг левой границы окна
            currWindow += 1; // Приращение счетчика окон
        }
        vecNumberOfWindows_[i] = currWindow; // Установка числа окон (без учета среднего)
    }
}

// Выделение памяти для поля типа ArrayStatCharacters и ArrayRegressionParams
template<typename T>
void AssociatedStatistics::allocateField(T& field){
    if (isEmpty()) return; // Если сигналов меньше двух
    field.resize(nSize_); // Параметры линейной регрессии
    int nMainWindows = vecNumberOfWindows_[indMainSignal_]; // Число окон в главном сигнале
    int k = 0; // Счетчик сигналов
    for (int i = 0; i != nSignal_; ++i){
        if (i == indMainSignal_) continue; // Пропуск главного сигнала
        field[k].resize(nMainWindows); // Параметры линейной регрессии
        int nCurrentWindows = vecNumberOfWindows_[i]; // Число окон в текущем сигнале
        for (int j = 0; j != nMainWindows; ++j){
            field[k][j].resize(nCurrentWindows); // Параметры линейной регрессии
        }
        k = k + 1;
    }
}

// Выделение памяти для всех полей
void AssociatedStatistics::allocateAllFields(){
    allocateField(regressionParams_); // Параметры линейной регрессии
    allocateField(distanceScatter_);  // Дистанция рассеяния
    allocateField(similarityCoeffs_); // Коэффициенты подобия сигналов
    allocateField(amplitudeScatter_); // Амплитуда рассеяния
    allocateField(noiseCoeffs_);      // Коэффициенты шума
}

// Установление соответствия статистик и сигналов
void AssociatedStatistics::findCorrespondence(){
    int k = 0; // Счетчик сигналов
    for (int i = 0; i != nSignal_; ++i){
        if (i == indMainSignal_) continue; // Пропуск главного сигнала
        vecIndCorrespond_[k] = i;
        k = k + 1;
    }
}

// -------------------------------------------------------------------------------------------------------------
