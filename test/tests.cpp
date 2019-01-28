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

void testStatistics() {
    int volume = 12;
    QString testPath = "/home/qinterfly/Library/SignalProcessing/EvalSig/test/";
    DataSignal obj1(testPath, "ОП 182 1с ку.txt");
    DataSignal obj2(testPath, "ОП 182 2с ку.txt");
    QVector<DataSignal> vecDataSignals;
    vecDataSignals.reserve(volume);
    vecDataSignals.push_back(obj1);
    vecDataSignals.push_back(obj2);
    Statistics stat(vecDataSignals, 365, 0.5);
    qDebug();
}
