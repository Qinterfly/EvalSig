#ifndef DATASIGNAL_H
#define DATASIGNAL_H

#include <QVector>
#include <QString>
#include "FileOperate.h"
#include "PropertyDataSignal.h"

struct DataSignal{
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
    double operator[](int) const;
    // Пользовательские методы
    int size() const; // Получение длины сигнала
    bool isEmpty() const; // Проверка на пустоту сигнала
    double mean() const; // Получение среднего значения сигнала
    QVector<double> const& getData() const; // Получение сигнала без свойств
    PropertyDataSignal const& getProperty() const; // Получение всех свойств
    QString getName() const; // Получение имени сигнала
    double convertCountToTime(int count) const; // Перевести номер отсчета в время
    // Файловые методы
    int readDataFile(QString const& path, QString const& fileName); // Чтение файла с данными
    int writeDataFile(QString const& path, QString const& fileName); // Запись файла с данными
private:
    PropertyDataSignal property; // Свойства сигнала
    QVector<double> data_;      // Временной сигнал
};

#endif // DATASIGNAL_H
