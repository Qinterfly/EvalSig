#include <QApplication>
#include "MainWindow.h"

#define testApp

#ifdef testApp
    #include "test/Tests.h"
    int main()
    {
        Tests::numericalFunctions();
//        Tests::dataSignal();
//        Tests::statistics();
//        Tests::divisionDataSignal();
//        Tests::associatedStatistics();
        return 0;
    }
#else
    int main(int argc, char * argv[])
    {
        QApplication app(argc, argv);
        MainWindow win;
        win.show();
        return app.exec();
    }
#endif
