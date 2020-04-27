#ifndef PROPERTYDATASIGNAL_H
#define PROPERTYDATASIGNAL_H

#include <QString>

// Закрытое поле класса DataSignal
struct PropertyDataSignal {  // Контейнер для свойств
    QString path_;           // Путь к файлу
    QString fileName_;       // Имя файла
    QString dateAndTime_;    // Дата и время записи сигнала
    QString measureObject_;  // Объект измерения
    QString measurePoint_;   // Точка установки датчика
    QString currentCount_;   // Текущие отсчеты
    double temperature_;     // Температура
    QString sensorType_;     // Тип датчика
    double physicalFactor_;  // Физический коэффициент
    QString measureUnit_;    // Единица измерения
    double scanPeriod_;         // Период опроса датчика
    QString characteristic_; // Характеристика
    int nCount_ = 0;         // Количество отсчетов
    bool isSpectrum = false; // Флаг спектра
};

#endif // PROPERTYDATASIGNAL_H
