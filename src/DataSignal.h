#ifndef DATASIGNAL_H
#define DATASIGNAL_H

#include <QVector>
#include <QString>
#include <QFile>
#include <QFileInfo>
#include <QDebug>

struct PropertyDataSignal { // Контейнер для свойств
    QString path_;          // Путь к файлу
    QString fileName_;      // Имя файла
    QString dateAndTime_;   // Дата и время записи сигнала
    QString measureObject_; // Объект измерения
    QString measurePoint_;  // Точка установки датчика
    QString currentCount_;  // Текущие отсчеты
    double temperature_;    // Температура
    QString sensorType_;    // Тип датчика
    double physicalCoeff_;  // Физический коэффициент
    QString measureUnit_;   // Единица измерения
    double scanPeriod_;     // Период опроса датчика
    QString characterisic_; // Характеристика
    int nCount_ = 0;        // Количество отсчетов
};

struct DataSignal{
    // Конструкторы и деструктор
    DataSignal() {}
    DataSignal(QString const& path, QString const& fileName); // Конструктор от пути и имени файла
    DataSignal(DataSignal const&); // Копирующий конструктор
    DataSignal(DataSignal &&); // Перемещающий конструктор
    ~DataSignal() = default;
    // Операторы
    DataSignal& operator=(DataSignal const&);
    bool operator==(DataSignal const&) const;
    bool operator!=(DataSignal const&) const;
    double operator[](int) const;
    // Пользовательские методы
    int size() const; // Получение длины сигнала
    QVector<double> getData() const; // Получение сигнала без свойств
    PropertyDataSignal getProperty() const; // Получение свойств
    // Файловые методы
    void readDataFile(QString const& path, QString const& fileName); // Чтение файла с данными
    bool writeDataFile(QString const& path, QString const& fileName); // Запись файла с данными
private:
    PropertyDataSignal property; // Свойства сигнала
    QVector<double> data_;      // Временной сигнал
};

#endif // DATASIGNAL_H
