#ifndef PROPERTYDATASIGNAL_H
#define PROPERTYDATASIGNAL_H

#include <QString>

// Закрытое поле класса DataSignal
struct PropertyDataSignal { // Контейнер для свойств
    QString path_;          // Путь к файлу
    QString fileName_;      // Имя файла
    QString dateAndTime_;   // Дата и время записи сигнала
    QString measureObject_; // Объект измерения
    QString measurePoint_;  // Точка установки датчика
    QString currentCount_;  // Текущие отсчеты
    double temperature_;    // Температура
    QString sensorType_;    // Тип датчика
    double physicalFactor_; // Физический коэффициент
    QString measureUnit_;   // Единица измерения
    int scanPeriod_;     // Период опроса датчика
    QString characterisic_; // Характеристика
    int nCount_ = 0;        // Количество отсчетов
};

#endif // PROPERTYDATASIGNAL_H
