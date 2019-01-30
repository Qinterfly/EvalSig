#include "tests.h"

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
    DataSignal obj1(testPath, "Short1.txt"); // Длина == 200
    DataSignal obj2(testPath, "Short2.txt"); // Длина == 200
    DataSignal obj3(testPath, "Short3.txt"); // Длина == 200
    QVector<DataSignal> vecDataSignals;
    vecDataSignals.reserve(volume);
    vecDataSignals.push_back(obj1);
    vecDataSignals.push_back(obj2);
    Statistics stat(vecDataSignals, 200, 0.); // Создание объекта статистик
    // Добавление сигнала
    stat.addSignal(vecDataSignals, obj3);
    // Удаление всех сигналов
    stat.removeSignal(vecDataSignals, 2);
    stat.removeSignal(vecDataSignals, 1);
    stat.removeSignal(vecDataSignals, 0);
    // Добавление сигналов
    stat.addSignal(vecDataSignals, obj1);
    stat.addSignal(vecDataSignals, obj2);
    stat.addSignal(vecDataSignals, obj3);
    qDebug();
}