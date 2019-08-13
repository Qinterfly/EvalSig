#ifndef PARTSSIGNAL_H
#define PARTSSIGNAL_H

#include <QVector>
#include "core/DataSignal.h"

using partsDouble = QVector< QVector<double> >;
using partsInt = QVector< QVector<int> >;

// Закрытое поле класса DivisionDataSignal

struct PartsObject {
    PartsObject() = default;
    ~PartsObject() = default;
    // Изменение размеров
    void resizeAll(int nLevels);                 // Все поля по числу уровней
    void resizeMain(int levelInd, int lenLevel); // Время + данные + флаги для заданного уровня
    void resizeInd(int levelInd);                // Изменение размера индексов по числу фрагментов

    partsInt time_;                 // Время
    partsDouble data_;              // Данные
    partsDouble derivative_;        // Производные
    partsInt flags_;                // Флаги концов фрагментов
    partsInt ind_;                  // Индексы концов фрагментов
    QVector<int> lengthLevels_;     // Длины уровней
    QVector<int> nFragmentLevels_;  // Число фрагментов на уровнях
    int nLevels_ = 0;               // Число уровней
};

struct PartsSignal : public PartsObject {
    PartsSignal(DataSignal const& signal);
    ~PartsSignal() = default;
    void constructByImage(PartsSignal const& other); // Создание частей сигнала по образу

    DataSignal const& signal_;      // Ссылка на исходный сигнал
};

struct PartsMonotone : public PartsObject {
    PartsMonotone(PartsSignal const& baseObjectParts, PartsSignal const& baseSeparationParts);
    ~PartsMonotone() = default;

    PartsSignal const& baseObjectParts_;       // Ссылка на базовые части с сигналом
    PartsSignal const& baseSeparationParts_;   // Ссылка на базовые части для деления по уровням
};

#endif // PARTSSIGNAL_H
