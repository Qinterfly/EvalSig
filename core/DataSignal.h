#ifndef DATASIGNAL_H
#define DATASIGNAL_H

#include <QVector>
#include <QString>
#include <functional>
#include "FileOperate.h"
#include "PropertyDataSignal.h"

struct DataSignal {
    // Конструкторы и деструктор
    DataSignal() = default;
    DataSignal(QString const& path, QString const& fileName); // Конструктор от пути и имени файла
    DataSignal(QVector<double> const& someData, PropertyDataSignal const& someProperty);
    DataSignal(DataSignal const&); // Копирующий конструктор
    DataSignal(DataSignal &&); // Перемещающий конструктор
    ~DataSignal(); // Деструктор
    // Операторы
    DataSignal& operator=(DataSignal const&);
    bool operator==(DataSignal const&) const;
    bool operator!=(DataSignal const&) const;
    double operator[](int index) const { return data_[index]; }
    // Пользовательские методы
    int size() const { return property.nCount_; }  // Получение длины сигнала
    bool isEmpty() const { return !size(); } // Проверка на пустоту сигнала
    double mean() const; // Получение среднего значения сигнала
    void normalize(QString const& option); // Нормализация сигнала
    QVector<double> const& getData() const { return data_; } // Получение сигнала без свойств
    PropertyDataSignal const& getProperty() const { return property; } // Получение всех свойств
    QString getName() const { return property.fileName_; } // Получение имени сигнала
    double convertCountToTime(int count) const { return count * 1.0e-6 * property.scanPeriod_; } // Перевести номер отсчета в время
    double nyquistFrequency() const { return property.scanPeriod_ * 1.0e-3 / 2.0; } // Частота Найквиста
    // Файловые методы
    int readDataFile(QString const& path, QString const& fileName); // Чтение файла с данными
    int writeDataFile(QString const& path, QString const& fileName, int leftInd = 0, int rightInd = -1) const; // Запись временного сигнала на выбранном участке
private:
    PropertyDataSignal property; // Свойства сигнала
    QVector<double> data_;       // Временной сигнал
};

// Вспомогательные функции

// Поиск минимума-максимума в векторе
double minMaxVec(QVector<double> const& vecD, std::function<bool(double, double)> && orderFun);

// Вычисление среднего для вектора
double meanVec(QVector<double> const& vecD);

// Нормализация вектора
void normalizeVec(QVector<double> & vecD);

#endif // DATASIGNAL_H
