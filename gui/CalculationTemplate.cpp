#include <QDir>
#include <QFile>
#include <QDataStream>
#include "gui/CalculationTemplate.h"
#include "core/FileOperate.h"

// ---- Расчетный шаблон для обработки файлов ------------------------------------------------------------------

// Пользовательские методы

// Установка параметров статистик
void CalculationTemplate::setStatParams(QPair <int, int> const& estimationBoundaries, int widthWindow, int shiftWindow){
    estimationBoundaries_ = estimationBoundaries; // Границы расчета
    widthWindow_ = widthWindow; // Ширина окна
    shiftWindow_ = shiftWindow; // Смещение окна
}

// Вставка данных поля окна
void CalculationTemplate::addWindowData(QString const& windowName, QString const& dataName, QVariant const& data){
    // Проверка существования переданного окна
    if (!windowsData_.contains(windowName)){
        windowsData_.insert(windowName, WindowData());
        sequenceOfWindows_.push_back(windowName);
    }
    windowsData_[windowName].insert(dataName, data);
}

// Удаление действия
void CalculationTemplate::removeWindow(int index){
    sequenceOfWindows_.removeAt(index);
}

// Очистка шаблона
void CalculationTemplate::clear(){
    windowsData_.clear();
    sequenceOfWindows_.clear();
    estimationBoundaries_ = {0, 0};
    widthWindow_ = 0;
    shiftWindow_ = 0;
}

// Запись в бинарный файл
int CalculationTemplate::write(QString const& path, QString const& fileName) const{
    QDir::setCurrent(path); // Выбираем директорию для сохранения
    QFile file(fileName); // Инициализация файла для записи
    QString fileFullPath = path + fileName; // Полное имя файла
    if (!checkFile(fileFullPath, "write")){ return -1; } // Обработка ошибок
    file.open(QIODevice::WriteOnly); // Открытие файла для записи
    QDataStream outputStream(&file); // Создание потока для записи
    // Запись информации параметров окна для вычисления статистик
    outputStream << 1; // Версия
    outputStream << estimationBoundaries_.first << estimationBoundaries_.second; // Расчетные границы
    outputStream << widthWindow_; // Ширина окна
    outputStream << shiftWindow_; // Смещение окна
    // Запись информации о шаблоне
    int nWindows = sequenceOfWindows_.size();
    outputStream << nWindows; // Число графических окон
    // Прописываем имя и длину данных для каждого окна
    for (QString const& windowName : sequenceOfWindows_)
        outputStream << windowName << windowsData_[windowName].size();
    // Запись данных для каждого окна
    for (QString const& windowName : sequenceOfWindows_){
        QHash<QString, QVariant>::const_iterator dataIterator = windowsData_[windowName].cbegin();
        QHash<QString, QVariant>::const_iterator endDataIterator = windowsData_[windowName].cend();
        for (; dataIterator != endDataIterator; ++dataIterator){
            outputStream << QString(dataIterator.key()) << dataIterator.value();
        }
    }
    file.close();
    return outputStream.status();
}

// Чтение бинарного файла
int CalculationTemplate::read(QString const& path, QString const& fileName) const {
    QDir::setCurrent(path); // Выбираем директорию для сохранения
    QFile file(fileName); // Инициализация файла для чтения
    QString fileFullPath = path + fileName; // Полное имя файла
    if (!checkFile(fileFullPath, "read")){ return -1; } // Обработка ошибок
    file.open(QIODevice::ReadOnly); // Открытие файла для чтения
    QDataStream outputStream(&file); // Создание потока для чтения
    // Чтение файла
    int version;
    outputStream >> version;
    file.close(); // Закрытие файла
    return 0;
}

// -------------------------------------------------------------------------------------------------------------
