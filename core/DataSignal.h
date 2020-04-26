#ifndef DATASIGNAL_H
#define DATASIGNAL_H

#include <QVector>
#include <QString>
#include <functional>
#include "FileOperate.h"
#include "PropertyDataSignal.h"

enum NormalizeOption{ FIRST, MEAN }; // Опции нормировки
enum ExtremaOption{ MIN, MAX }; // Опции отыскания экстремума

class DataSignal {
public:
    // Конструкторы и деструктор
    DataSignal() = default;
    DataSignal(QString const& path, QString const& fileName, int shift = 0); // Конструктор от пути и имени файла
    DataSignal(QVector<double> const& someData, PropertyDataSignal const& someProperty);
    DataSignal(DataSignal const& other, int leftInd = 0, int rightInd = -1); // Копирующий конструктор по заданной области
    DataSignal(DataSignal &&) noexcept; // Перемещающий конструктор для всего сигнала
    DataSignal(QVector<double> && someData, PropertyDataSignal && someProperty) noexcept; // Перемещающий конструктор для данных сигнала
    ~DataSignal() = default; // Деструктор
    // Операторы
    DataSignal& operator=(DataSignal const&);
    bool operator==(DataSignal const&) const;
    bool operator!=(DataSignal const&) const;
    double operator[](int index) const { return data_[index]; }
    // Пользовательские методы
    int size() const { return property.nCount_; }  // Получение длины сигнала
    bool isEmpty() const { return !size(); } // Проверка на пустоту сигнала
    bool isSpectrum() const { return property.isSpectrum; } // Проверка является ли сигнал спектром
    double mean() const; // Получение среднего значения сигнала
    void normalize(NormalizeOption option); // Нормализация сигнала
    QVector<double> const& getData() const { return data_; } // Получение сигнала без свойств
    QVector<double> getData(int leftInd, int rightInd) const { return data_.mid(leftInd, rightInd - leftInd + 1); } // Получение среза сигнала
    PropertyDataSignal const& getProperty() const { return property; } // Получение всех свойств
    QString getName() const { return property.fileName_; } // Получение имени сигнала
    double convertCountToTime(int count) const { return count * 1e-6 * property.scanPeriod_; } // Перевести номер отсчета в время
    double timeDuration() const { return convertCountToTime(size()); } // Длительность записи в секундах
    double nyquistFrequency() const { return property.scanPeriod_ * 1.0e-2 / 2.0; } // Частота Найквиста
    // Файловые методы
    int readDataFile(QString const& path, QString const& fileName, int shift = 0); // Чтение файла с данными
    int writeDataFile(QString const& path, QString const& fileName, int leftInd = 0, int rightInd = -1) const; // Запись временного сигнала на выбранном участке
    // Изменение параметров сигнала
    void setFileName(QString const& fileName);             // Имя файла
    void setDateAndTime(QString const& dateAndTime);       // Время записи
    void setMeasureObject(QString const& measureObject);   // Объект измерения
    void setMeasurePoint(QString const& measurePoint);     // Точка установки датчика
    void setTemperature(double temperature);               // Температура
    void setSensorType(QString const& sensorType);         // Тип датчика
    void setPhysicalFactor(double physicalFactor);         // Физический коэффициент
    void setMeasureUnit(QString const& measureUnit);       // Единицы измерения
    void setScanPeriod(int scanPeriod);                    // Период опроса датчика
    void setCharacteristic(QString const& characteristic); // Характеристика
private:
    PropertyDataSignal property; // Свойства сигнала
    QVector<double> data_;       // Временной сигнал
};

// Вспомогательные функции

// Поиск минимума-максимума в векторе
QPair<double, double> minMaxVec(QVector<double> const& vec, int leftInd = 0, int rightInd = -1);

// Поиск максимума в векторе
double maxVec(QVector<double> const& vec, int leftInd = 0, int rightInd = -1);

// Вычисление среднего для вектора
double meanVec(QVector<double> const& vec, int leftInd = 0, int rightInd = -1);

// Нормализация вектора
void normalizeVec(QVector<double> & vec, NormalizeOption option, int leftInd = 0, int rightInd = -1);

// Поиск сигнала с минимальной-максимальной длительности записи
int minOrMaxByLength(QVector<DataSignal> const & vecDataSignal, ExtremaOption opt);

#endif // DATASIGNAL_H
