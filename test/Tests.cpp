#include <QDebug>
#include "Tests.h"
#include "core/NumericalFunctions.h"
#include "core/DivisionDataSignal.h"
#include "core/AssociatedStatistics.h"

// Проверка временных сигналов
void testDataSignal(){
   QString testPath = "/home/qinterfly/Library/SignalProcessing/EvalSig/test/";
   DataSignal obj1(testPath, "ОП 182 1с ку.txt");
   DataSignal obj2(testPath, "ОП 182 2с ку.txt");

   // Операторы
   Q_ASSERT(obj1 != obj2);
   Q_ASSERT(obj1 == obj1);
   obj1 = obj2;
   Q_ASSERT(obj1 == obj2);

   // Get
   auto t = obj1.getData();
   Q_ASSERT(t.size() == obj1.size());
   qDebug();
}

// Проверка статистических характеристик
void testStatistics() {
    int volume = 12;
    QString testPath = "/home/qinterfly/Library/SignalProcessing/EvalSig/test/";
    // Создание по готовым сигналам
    DataSignal obj1(testPath, "Short1.txt"); // Длина == 200
    DataSignal obj2(testPath, "Short2.txt"); // Длина == 200
    DataSignal obj3(testPath, "Short3.txt"); // Длина == 200
    DataSignal obj4(testPath, "Short4.txt"); // Длина == 193
    DataSignal obj5(testPath, "Short5.txt"); // Длина == 632

    QVector<DataSignal> vecDataSignals;
    vecDataSignals.reserve(volume);
    vecDataSignals.push_back(obj1);
    vecDataSignals.push_back(obj2);
    Statistics stat(vecDataSignals, 200, 200, 1, 200); // Создание объекта статистик
    // Добавление сигнала
    Q_ASSERT(!stat.addSignal(obj3));
    // Удаление всех сигналов
    Q_ASSERT(!stat.removeSignal(2));
    Q_ASSERT(!stat.removeSignal(1));
    Q_ASSERT(!stat.removeSignal(0));
    // Добавление сигналов
    Q_ASSERT(!stat.addSignal(obj1));
    Q_ASSERT(!stat.addSignal(obj2));
    Q_ASSERT(!stat.addSignal(obj3));
    // Переключение параметров окна
    stat.setWindowProperty(193, 50);
    stat.setWindowProperty(193, 193);
    // Добавление сигналов другой длины
    Q_ASSERT(!stat.addSignal(obj4));
    Q_ASSERT(!stat.addSignal(obj5));
    // Переключение параметров окна
    stat.setWindowProperty(512, 256);
    stat.setWindowProperty(1024, 1024);
    stat.setWindowProperty(2048, 2048);
    stat.setWindowProperty(10, 1);
    // Сохранение статистик
    Q_ASSERT(!stat.writeAllStatistics("/home/qinterfly/Library/SignalProcessing/EvalSig/test/save/"));
    // Удаление сигналов
    stat.removeSignal(4);
    stat.removeSignal(3);
    stat.removeSignal(2);
    stat.removeSignal(1);
    stat.removeSignal(0);
    qDebug();
}

// Проверка численных методов
void testNumericalFunctions(){
    QString testPath = "/home/qinterfly/Library/SignalProcessing/EvalSig/test/";
    DataSignal obj1(testPath, "Short2.txt"); // Длина == 200
    // Фильтрация
    DataSignal filt = bandpassFilter(obj1, HAMMING, 64, 0.5, {10, 20});
    Q_ASSERT(!filt.writeDataFile(testPath, "filtObj.txt"));
    // Вычисление спектра
    DataSignal psd = computePowerSpectralDensity(obj1, HAMMING, 64, 0.5, 512, 3);
    Q_ASSERT(!psd.writeDataFile(testPath, "psdObj.txt"));
    // Интегрирование
    QVector<DataSignal> integrVecObj = integrate(obj1, 1, -1);
    Q_ASSERT(!integrVecObj[0].writeDataFile(testPath, "integrObj.txt"));
    // Аппроксимация
    DataSignal approxObj = approximateSmoothSpline(obj1, 1e-4);
    Q_ASSERT(!approxObj.writeDataFile(testPath, "approxObj.txt"));
    // Линейная интерполяция
    DataSignal linInterpObj = interpolateLinear(obj1, 400);
    Q_ASSERT(!linInterpObj.writeDataFile(testPath, "interpLinObj.txt"));
    // Сплайн-интерполяция
    DataSignal splineInterpObj = interpolateSpline(obj1, {1, 200}, 1000);
    Q_ASSERT(!splineInterpObj.writeDataFile(testPath, "interpSplineObj.txt"));
}

// Проверка разбиения сигнала на уровни
void testDivisionDataSignal(){
    // #1
    QString testPath = "/home/qinterfly/Library/SignalProcessing/EvalSig/test/";
    DataSignal obj1(testPath, "Short2.txt"); // Длина == 200
    DataSignal dispObj1 = integrate(obj1, 2, 0.5)[1]; // Нахождение перемещений по сигналу ускорения
    // Нормализация данных
    obj1.normalize(FIRST);
    dispObj1.normalize(FIRST);
    DataSignal approxDispObj1 = approximateSmoothSpline(dispObj1, 0.5); // Аппроксимация перемещений
    DivisionDataSignal divSignal1(obj1, dispObj1, approxDispObj1, 0.02, 0.5, 0.2, 0.1, 1, -1);
    divSignal1.calculateLevels(); // Расчет
    divSignal1.calculatePowerSpectralDensity(HAMMING, 0.5, 1024, 3); // Нахождение плотности спектральной мощности
    // Сохранение результатов
    Q_ASSERT(!divSignal1.writeAll(testPath + "saveLevels/"));
    Q_ASSERT(!divSignal1.writeSupport(testPath, "dispOp182.txt"));
    Q_ASSERT(!divSignal1.writeApproxSupport(testPath, "approxDispOp182.txt"));
    // #2
    DataSignal obj2(testPath, "ОП 182 1с ку.txt"); // Длина == 30061
    DataSignal dispObj2 = integrate(obj2, 2, 1e-7)[1]; // Нахождение перемещений по сигналу ускорения
    // Нормализация данных
    obj2.normalize(FIRST);
    dispObj2.normalize(FIRST);
    DataSignal approxDispObj2 = approximateSmoothSpline(dispObj2, 1e-7); // Аппроксимация перемещений
    DivisionDataSignal divSignal2(obj2, dispObj2, approxDispObj2, 15, 0.5, 0.2, 0.1, 1, -1);
    divSignal2.calculateLevels();
    divSignal2.calculatePowerSpectralDensity(HAMMING, 0.5, 1024, 3);
    Q_ASSERT(!divSignal2.writeAll(testPath + "saveLevels/"));
    Q_ASSERT(!divSignal2.writeSupport(testPath, "dispOp182.txt"));
    Q_ASSERT(!divSignal2.writeApproxSupport(testPath, "approxDispOp182.txt"));
}

// Проверка неизменяемых статистических характеристик
void testAssociatedStatistics() {
    int volume = 12;
    QString testPath = "/home/qinterfly/Library/SignalProcessing/EvalSig/test/";
    // Создание по готовым сигналам
    DataSignal obj1(testPath, "Short1.txt"); // Длина == 200
    DataSignal obj2(testPath, "Short2.txt"); // Длина == 200
    DataSignal obj3(testPath, "Short3.txt"); // Длина == 200
    DataSignal obj4(testPath, "Short4.txt"); // Длина == 193
    DataSignal obj5(testPath, "Short5.txt"); // Длина == 632
    QVector<DataSignal> vecDataSignals;
    vecDataSignals.reserve(volume);
    vecDataSignals.push_back(obj1);
    vecDataSignals.push_back(obj2);
    vecDataSignals.push_back(obj3);
    vecDataSignals.push_back(obj4);
    vecDataSignals.push_back(obj5);
    AssociatedStatistics stat(vecDataSignals, 80, 25, 10, 0); // Создание объекта статистик
    Q_ASSERT(!stat.computeStatistics());
}
