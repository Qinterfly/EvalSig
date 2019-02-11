#include "TimeWindowProperty.h"

// ---- Контейнер свойств временного окна ----------------------------------------------------------------------

// Конструктор TimeWindowProperty
TimeWindowProperty::TimeWindowProperty(int width, double overlapFactor, int sizeSignals)
    : width_(width), overlapFactor_(overlapFactor) { calcWindowParams(sizeSignals); }

// Расчет параметров окна
void TimeWindowProperty::calcWindowParams(int sizeSignals){
    if (width_ == 0) { // Обработка исключения окна нулевой ширины
        nWindows_ = 0; shiftWindow_ = 0;
        return;
    }
    nWindows_ = qCeil(sizeSignals / ( width_ * (1 - overlapFactor_) ) ); // Число окон
    shiftWindow_ = qCeil( width_ * (1 - overlapFactor_) ); // Смещение окна по времени
}

// Запись параметров окна
int TimeWindowProperty::writeWindowParams(QString const& path, QString const& fileName){
    QString fileFullPath = path + fileName; // Полный путь к файлу
    QFile file(fileFullPath); // Инициализация файла для записи
    if (!checkFile(fileFullPath, "write")){ return -1; } // Обработка ошибок
    file.open(QIODevice::WriteOnly | QIODevice::Text); // Открытие файла для записи
    QTextStream outputStream(&file); // Создание потока для записи
    outputStream.setCodec("cp1251"); // Кодировка CP1251
    outputStream << QString("Ширина временного окна = ") << width_ << endl;
    outputStream << QString("Коэффициент перекрытия окон = ") << overlapFactor_ << endl;
    outputStream << QString("Число окон = ") << nWindows_ << endl;
    outputStream << QString("Шаг окон = ") << shiftWindow_ << endl;
    file.close(); // Закрытие файла
    return 0;
}

// -------------------------------------------------------------------------------------------------------------
