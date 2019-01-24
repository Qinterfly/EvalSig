#include "mainwindow.h"
#include <QApplication>
#include "datasignal.h"

int main(int argc, char *argv[])
{
//    QApplication a(argc, argv);
//    MainWindow w;
//    w.show();
//    return a.exec();
    DataSignal obj("/home/qinterfly/Library/EvalSig/", "Test.txt");
    return 0;
}
