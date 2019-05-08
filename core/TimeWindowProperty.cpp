#include <QFile>
#include <QtMath>
#include <QTextStream>
#include "TimeWindowProperty.h"

// ---- Контейнер свойств временного окна ----------------------------------------------------------------------

// Конструктор TimeWindowProperty
TimeWindowProperty::TimeWindowProperty(int width, int shiftWindow, QPair<int, int> const& estimationBoundaries, int minSizeSignals)
    : width_(width), shiftWindow_(shiftWindow) { calcWindowParams(estimationBoundaries, minSizeSignals); }

// Расчет параметров окна
void TimeWindowProperty::calcWindowParams(QPair<int, int> const& estimationBoundaries, int minSizeSignals){
    // Обработка исключения окна нулевой ширины
    if (width_ == 0) {
        nWindows_ = 0; shiftWindow_ = 0;
        return;
    }
    // Подсчет числа окон
    int currWindow = 0; // Номер текущего окна
        // Пока текущая левая граница не достигнет конца правой расчетной границы
    for (int s = estimationBoundaries.first - 1; s < estimationBoundaries.second && s < minSizeSignals; ){
        int currRightBound = width_;
        if (currRightBound + s > minSizeSignals) // Проверка правой границы
            currRightBound = minSizeSignals - s;
        // Сдвиг
        s += shiftWindow_; // Сдвиг левой границы окна
        currWindow += 1; // Приращение счетчика окон
    }
    nWindows_ = currWindow; // Установка числа окон (без учета среднего)
}

// Запись параметров окна
int TimeWindowProperty::writeWindowParams(QString const& path, QString const& fileName) const {
    QString fileFullPath = path + fileName; // Полный путь к файлу
    QFile file(fileFullPath); // Инициализация файла для записи
    if (!checkFile(fileFullPath, "write")){ return -1; } // Обработка ошибок
    file.open(QIODevice::WriteOnly | QIODevice::Text); // Открытие файла для записи
    QTextStream outputStream(&file); // Создание потока для записи
    outputStream.setCodec("cp1251"); // Кодировка CP1251
    outputStream << QString("Ширина временного окна = ") << width_ << endl;
    outputStream << QString("Число временных окон = ") << nWindows_ << endl;
    outputStream << QString("Шаг временнего окна = ") << shiftWindow_ << endl;
    file.close(); // Закрытие файла
    return 0;
}

// -------------------------------------------------------------------------------------------------------------
