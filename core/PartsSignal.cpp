#include "PartsSignal.h"

// Конструктор
PartsSignal::PartsSignal(DataSignal const& signal) : signal_(signal) { }

// Изменение размеров всех полей по числу уровней
void PartsSignal::resizeAll(int nLevels){
    time_.resize(nLevels);             // Время
    data_.resize(nLevels);             // Данные
    flags_.resize(nLevels);            // Флаги концов фрагментов
    ind_.resize(nLevels);              // Индексы концов фрагментов
    lengthLevels_.resize(nLevels);     // Длины уровней
    nFragmentLevels_.resize(nLevels);  // Число фрагментов на уровнях
}

// Изменение размеров время, сигнала и флагов для заданного уровня
void PartsSignal::resizeMain(int levelInd, int lenLevel){
    time_[levelInd].resize(lenLevel);    // Время
    data_[levelInd].resize(lenLevel);    // Данные
    flags_[levelInd].resize(lenLevel);   // Флаги концов фрагментов
}

// Изменение размера индексов по числу фрагментов
void PartsSignal::resizeInd(int levelInd){
    ind_[levelInd].resize(nFragmentLevels_[levelInd]);
}
