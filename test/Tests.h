#ifndef TESTS_H
#define TESTS_H

#include "core/DataSignal.h"
#include "core/Statistics.h"

class Tests
{
public:
    static void dataSignal(); // Проверка сигналов
    static void statistics(); // Проверка статистик
    static void numericalFunctions(); // Проверка численных методов
    static void divisionDataSignal(); // Проверка разбиения сигнала на уровни
    static void associatedStatistics(); // Проверка неизменяемых статистических характеристик
};

#endif // TESTS_H
