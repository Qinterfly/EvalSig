#include "PartsObject.h"

// -- BaseParts --
// Изменение размеров всех полей по числу уровней
void PartsObject::resizeAll(int nLevels){
    time_.resize(nLevels);             // Время
    data_.resize(nLevels);             // Данные
    derivative_.resize(nLevels);       // Производные
    flags_.resize(nLevels);            // Флаги концов фрагментов
    ind_.resize(nLevels);              // Индексы концов фрагментов
    lengthLevels_.resize(nLevels);     // Длины уровней
    nFragmentLevels_.resize(nLevels);  // Число фрагментов на уровнях
    nLevels_ = nLevels;                // Число уровней
}

// Изменение размеров время, сигнала и флагов для заданного уровня
void PartsObject::resizeMain(int levelInd, int lenLevel){
    time_[levelInd].resize(lenLevel);       // Время
    data_[levelInd].resize(lenLevel);       // Данные
    flags_[levelInd].resize(lenLevel);      // Флаги концов фрагментов
    derivative_[levelInd].resize(lenLevel); // Производные
}

// Изменение размера индексов по числу фрагментов
void PartsObject::resizeInd(int levelInd){
    ind_[levelInd].resize(nFragmentLevels_[levelInd]);
}

// -- PartsSignal --
PartsSignal::PartsSignal(DataSignal const& signal) : signal_(signal) { }

// Создание частей сигнала по образу
void PartsSignal::constructByImage(PartsSignal const& other){
    if (this == &other) return; // Проверка идентичности объекта
    nLevels_ = other.nLevels_; // Число уровней
    resizeAll(nLevels_); // Выделение памяти для всех полей
    int lenLevel = 0; // Длина текущего уровня
    int nFragments = 0; // Число фрагментов на уровне
    for (int i = 0; i != nLevels_; ++i){
        lenLevel = other.lengthLevels_[i];
        nFragments = other.nFragmentLevels_[i];
        resizeMain(i, lenLevel); // Изменение размеров основных полей
        // Копирование основных полей
        for (int j = 0; j != lenLevel; ++j){
            time_[i][j] = other.time_[i][j];        // Время
            data_[i][j] = signal_[time_[i][j] - 1]; // Данные
            flags_[i][j] = other.flags_[i][j];      // Флаги
        }
        // Копирование вспомогательных полей
        lengthLevels_[i] = lenLevel;            // Длины уровней
        nFragmentLevels_[i] = nFragments;       // Число фрагментов
        resizeInd(i);                           // Выделение памяти под индексы
        for (int j = 0; j != nFragments; ++j)   // Копирование индексов
            ind_[i][j] = other.ind_[i][j];
    }
}

// -- PartsMonotone --
PartsMonotone::PartsMonotone(PartsSignal const& baseParts) : baseParts_(baseParts) { }

