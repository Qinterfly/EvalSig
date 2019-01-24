#include "mainwindow.h"
#include <QApplication>
#include "datasignal.h"

int main(int argc, char *argv[])
{
//    QApplication a(argc, argv);
//    MainWindow w;
//    w.show();
//    return a.exec();
    DataSignal obj1("/home/qinterfly/Library/SignalProcessing/EvalSig/", "ОП 182 1с ку.txt");
    DataSignal obj2("/home/qinterfly/Library/SignalProcessing/EvalSig/", "ОП 182 2с ку.txt");
    return 0;
}
