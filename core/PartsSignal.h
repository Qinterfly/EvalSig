#ifndef PARTSSIGNAL_H
#define PARTSSIGNAL_H

#include <QVector>
#include "core/DataSignal.h"

using partsDouble = QVector< QVector<double> >;
using partsInt = QVector< QVector<int> >;

// Закрытое поле класса DivisionDataSignal
struct PartsSignal
{
    PartsSignal(DataSignal const& signal);
    ~PartsSignal() = default;
    // Изменение размеров
    void resizeAll(int nLevels);                 // Все поля по числу уровней
    void resizeMain(int levelInd, int lenLevel); // Время + данные + флаги для заданного уровня
    void resizeInd(int levelInd);                // Изменение размера индексов по числу фрагментов

    partsInt time_;                 // Время
    partsDouble data_;              // Данные
    partsInt flags_;                // Флаги концов фрагментов
    partsInt ind_;                  // Индексы концов фрагментов
    QVector<int> lengthLevels_;     // Длины уровней
    QVector<int> nFragmentLevels_;  // Число фрагментов на уровнях
    DataSignal const& signal_;      // Ссылка на исходный сигнал
};

#endif // PARTSSIGNAL_H
