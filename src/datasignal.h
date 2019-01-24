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
    int nCount_;            // Количество отсчетов
};

struct DataSignal{
    DataSignal(QString const& path, QString const& fileName);
    void readDataFile(QString const& path, QString const& fileName); // Чтение файла с данными
private:
    PropertyDataSignal property; // Свойства сигнала
    QVector<double> data_;      // Временной сигнал
};

#endif // DATASIGNAL_H
