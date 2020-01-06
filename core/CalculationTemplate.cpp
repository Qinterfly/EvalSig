#include <QDir>
#include <QFile>
#include <QDataStream>
#include <QDateTime>
#include "core/CalculationTemplate.h"
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
        if (!sequenceOfWindows_.contains(windowName))
            sequenceOfWindows_.push_back(windowName);
    }
    windowsData_[windowName].insert(dataName, data);
}

// Удаление окна
void CalculationTemplate::removeWindow(int index){
    windowsData_.remove(sequenceOfWindows_[index]);
    sequenceOfWindows_.removeAt(index);
}

// Очистка шаблона
void CalculationTemplate::clear(){
    windowsData_.clear();
    sequenceOfWindows_.clear();
    estimationBoundaries_ = {0, 0}; // Границы расчета
    widthWindow_ = 0; // Ширина окна
    shiftWindow_ = 0; // Смещение окна
    version_ = DEFAULT_VERSION; // Версия шаблона
    date_ = ""; // Дата
    note_ = ""; // Примечание к шаблону
}

// Запись в бинарный файл
int CalculationTemplate::write(QString const& path, QString const& fileName) {
    QDir::setCurrent(path); // Выбираем директорию для сохранения
    QFile file(fileName); // Инициализация файла для записи
    QString fileFullPath = path + fileName; // Полное имя файла
    if (!checkFile(fileFullPath, "write")){ return -1; } // Обработка ошибок
    file.open(QIODevice::WriteOnly); // Открытие файла для записи
    QDataStream outputStream(&file); // Создание потока для записи
    date_ = QDateTime::currentDateTime().toString("dd MMMM yyyy hh:mm:ss");
    // Служебная информация
    outputStream << version_; // Версия
    outputStream << date_; // Версия
    outputStream << note_; // Версия
    // Запись информации параметров окна для вычисления статистик
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
        for (; dataIterator != endDataIterator; ++dataIterator)
            outputStream << QString(dataIterator.key()) << dataIterator.value();
    }
    file.close();
    return outputStream.status();
}

// Чтение бинарного файла
int CalculationTemplate::read(QString const& path, QString const& fileName) {
    QDir::setCurrent(path); // Выбираем директорию для сохранения
    QFile file(fileName); // Инициализация файла для чтения
    QString fileFullPath = path + fileName; // Полное имя файла
    if (!checkFile(fileFullPath, "read")){ return -1; } // Обработка ошибок
    file.open(QIODevice::ReadOnly); // Открытие файла для чтения
    QDataStream inputStream(&file); // Создание потока для чтения
    // Чтение файла
    clear(); // Очистка шаблона
    // Служебная информация
    inputStream >> version_;
    inputStream >> date_;
    inputStream >> note_;
    if (version_ > DEFAULT_VERSION) return -1; // Если версия не поддерживается
    // Параметры окна для вычисления статистик
    inputStream >> estimationBoundaries_.first >> estimationBoundaries_.second; // Расчетные границы
    inputStream >> widthWindow_; // Ширина окна
    inputStream >> shiftWindow_; // Смещение окна
    // Информация о шаблоне
    int nWindows = 0;
    inputStream >> nWindows; // Число графических окон
    // Имя и длина данных для каждого окна
    QVector<int> sizeOfWindows(nWindows);
    QString tString;
    for (int i = 0; i != nWindows; ++i){
        inputStream >> tString >> sizeOfWindows[i];
        sequenceOfWindows_.push_back(tString);
    }
    // Данные для каждого окна
    QVariant tVariant;
    for (int i = 0; i != nWindows; ++i){
        int currSize = sizeOfWindows[i];
        QString const& windowName = sequenceOfWindows_[i];
        for (int j = 0; j != currSize; ++j){
            inputStream >> tString >> tVariant;
            addWindowData(windowName, tString, tVariant);
        }
    }
    file.close(); // Закрытие файла
    return inputStream.status();
}

// -------------------------------------------------------------------------------------------------------------
