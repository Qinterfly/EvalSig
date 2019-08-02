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

// -- PartsMonotone --
PartsMonotone::PartsMonotone(PartsSignal const& baseParts) : baseParts_(baseParts) { }

