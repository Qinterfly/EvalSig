#include <QDebug>
#include "Tests.h"
#include "core/NumericalFunctions.h"

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
    qDebug();
}

// Проверка численных методов
void testNumericalFunctions(){
    QString testPath = "/home/qinterfly/Library/SignalProcessing/EvalSig/test/";
    DataSignal obj1(testPath, "Short1.txt"); // Длина == 200
    // Вычисление спектра
    DataSignal psd = computePowerSpectralDensity(obj1, "Хэмминга", 64, 0.5);
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
}
