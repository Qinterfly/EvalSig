#include <QDebug>
#include "Tests.h"
#include "core/NumericalFunctions.h"
#include "core/DivisionDataSignal.h"
#include "core/AssociatedStatistics.h"

static QString const testInputPath = "/home/qinterfly/Library/Projects/SignalProcessing/EvalSig/test/input/";
static QString const testOutputPath = "/home/qinterfly/Library/Projects/SignalProcessing/EvalSig/test/output/";

// Проверка временных сигналов
void Tests::dataSignal(){
   DataSignal obj1(testInputPath, "ОП 182 1с ку.txt");
   DataSignal obj2(testInputPath, "ОП 182 2с ку.txt");
   DataSignal obj3(testInputPath, "Short1.txt", 10);
   DataSignal obj4(testInputPath, "Short1.txt", -10);
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
void Tests::statistics() {
    int volume = 12;
    // Создание по готовым сигналам
    DataSignal obj1(testInputPath, "Short1.txt"); // Длина == 200
    DataSignal obj2(testInputPath, "Short2.txt"); // Длина == 200
    DataSignal obj3(testInputPath, "Short3.txt"); // Длина == 200
    DataSignal obj4(testInputPath, "Short4.txt"); // Длина == 193
    DataSignal obj5(testInputPath, "Short5.txt"); // Длина == 632

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
    Q_ASSERT(!stat.writeAllStatistics(testOutputPath));
    // Удаление сигналов
    stat.removeSignal(4);
    stat.removeSignal(3);
    stat.removeSignal(2);
    stat.removeSignal(1);
    stat.removeSignal(0);
    qDebug();
}

// Проверка численных методов
void Tests::numericalFunctions(){
    DataSignal obj1(testInputPath, "Short2.txt"); // Длина == 200
    // Фильтрация
    DataSignal filt = bandpassFilter(obj1, HAMMING, 64, 0.5, {10, 20});
    Q_ASSERT(!filt.writeDataFile(testOutputPath, "filtObj.txt"));
    // Вычисление спектра
    DataSignal psd = computePowerSpectralDensity(obj1, HAMMING, 64, 0.5, 512, 3);
    Q_ASSERT(!psd.writeDataFile(testOutputPath, "psdObj.txt"));
    // Интегрирование
    QVector<DataSignal> integrVecObj = integrateTrapz(obj1, 1, -1);
    Q_ASSERT(!integrVecObj[0].writeDataFile(testOutputPath, "integrObj.txt"));
    // Аппроксимация
    DataSignal approxObj = approximateSmoothSpline(obj1, 1e-4);
    Q_ASSERT(!approxObj.writeDataFile(testOutputPath, "approxObj.txt"));
    // Линейная интерполяция
    DataSignal linInterpObj = interpolateLinear(obj1, 400, false);
    Q_ASSERT(!linInterpObj.writeDataFile(testOutputPath, "interpLinObj.txt"));
    // Линейная интерполяция по внутреннему числу точек
    DataSignal linInnerInterpObj = interpolateLinear(obj1, 100, true);
    Q_ASSERT(!linInnerInterpObj.writeDataFile(testOutputPath, "interpInnerLinObj.txt"));
    // Сплайн-интерполяция по общему числу точек
    DataSignal splineInterpObj = interpolateSpline(obj1, {1, 200}, 1000, false);
    Q_ASSERT(!splineInterpObj.writeDataFile(testOutputPath, "interpSplineObj.txt"));
    // Сплайн-интерполяция по внутреннему числу точек
    DataSignal splineInnerInterpObj = interpolateSpline(obj1, {1, 200}, 48, true);
    Q_ASSERT(!splineInnerInterpObj.writeDataFile(testOutputPath, "interpInnerSplineObj.txt"));
    // Исключение выбросов
    DataSignal exclOut = excludeOutliers(obj1, 0.17);
    Q_ASSERT(!exclOut.writeDataFile(testOutputPath, "exclOutObj.txt"));
    // Построение огибающей
    DataSignal obj2(testInputPath, "20161119T105535_1_CM-311.txt");
    QPair<DataSignal, DataSignal> envelopesObj = constructEnvelope(obj2);
    Q_ASSERT(!envelopesObj.first.writeDataFile(testOutputPath, "lowerEnvelopeObj.txt"));
    Q_ASSERT(!envelopesObj.second.writeDataFile(testOutputPath, "upperEnvelopeObj.txt"));
    qDebug();
}

// Проверка разбиения сигнала на уровни
void Tests::divisionDataSignal(){
    // #1
    DataSignal obj1(testInputPath, "Short2.txt"); // Длина == 200
    DataSignal dispObj1 = integrateTrapz(obj1, 2, 0.5)[1]; // Нахождение перемещений по сигналу ускорения
    // Нормализация данных
    obj1.normalize(FIRST);
    dispObj1.normalize(FIRST);
    DataSignal approxDispObj1 = approximateSmoothSpline(dispObj1, 0.5); // Аппроксимация перемещений
    DivisionDataSignal divSignal1(obj1, dispObj1, approxDispObj1, 0.02, 0.5, 0.2, 0.1, 1, -1);
    divSignal1.calculateLevels(); // Расчет
    divSignal1.calculatePowerSpectralDensity(HAMMING, 0.5, 1024, 3); // Нахождение плотности спектральной мощности
    // Сохранение результатов
    Q_ASSERT(!divSignal1.writeAll(testOutputPath + "saveLevels/"));
    Q_ASSERT(!divSignal1.writeSupport(testOutputPath, "dispOp182.txt"));
    Q_ASSERT(!divSignal1.writeApproxSupport(testOutputPath, "approxDispOp182.txt"));
    // #2
    DataSignal obj2(testOutputPath, "ОП 182 1с ку.txt"); // Длина == 30061
    DataSignal dispObj2 = integrateTrapz(obj2, 2, 1e-7)[1]; // Нахождение перемещений по сигналу ускорения
    // Нормализация данных
    obj2.normalize(FIRST);
    dispObj2.normalize(FIRST);
    DataSignal approxDispObj2 = approximateSmoothSpline(dispObj2, 1e-7); // Аппроксимация перемещений
    DivisionDataSignal divSignal2(obj2, dispObj2, approxDispObj2, 15, 0.5, 0.2, 0.1, 1, -1);
    divSignal2.calculateLevels();
    divSignal2.calculatePowerSpectralDensity(HAMMING, 0.5, 1024, 3);
    Q_ASSERT(!divSignal2.writeAll(testOutputPath + "saveLevels/"));
    Q_ASSERT(!divSignal2.writeSupport(testOutputPath, "dispOp182.txt"));
    Q_ASSERT(!divSignal2.writeApproxSupport(testOutputPath, "approxDispOp182.txt"));
    qDebug();
}

// Проверка неизменяемых статистических характеристик
void Tests::associatedStatistics() {
    int volume = 12;
    // Создание по готовым сигналам
    DataSignal obj1(testInputPath, "Short1.txt"); // Длина == 200
    DataSignal obj2(testInputPath, "Short2.txt"); // Длина == 200
    DataSignal obj3(testInputPath, "Short3.txt"); // Длина == 200
    DataSignal obj4(testInputPath, "Short4.txt"); // Длина == 193
    DataSignal obj5(testInputPath, "Short5.txt"); // Длина == 632
    QVector<DataSignal> vecDataSignals;
    vecDataSignals.reserve(volume);
    vecDataSignals.push_back(obj1);
    vecDataSignals.push_back(obj2);
    vecDataSignals.push_back(obj3);
    vecDataSignals.push_back(obj4);
    vecDataSignals.push_back(obj5);
    AssociatedStatistics stat(vecDataSignals, 80, 25, 10, 0); // Создание объекта статистик
    Q_ASSERT(!stat.computeStatistics());
    Q_ASSERT(!stat.writeAllStatistics(testOutputPath + "AssociatedStat/"));
    qDebug();
}
