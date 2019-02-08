#include "DataSignal.h"

// Конструктор от пути и имени файла
DataSignal::DataSignal(QString const& path, QString const& fileName){ readDataFile(path, fileName); }
// Копирующий конструктор
DataSignal::DataSignal(DataSignal const& other) : property(other.property), data_(other.data_) { };
// Перемещающий конструктор
DataSignal::DataSignal(DataSignal && tmpOther) : property(tmpOther.property), data_(tmpOther.data_) { }
// Деструктор
DataSignal::~DataSignal() { property.nCount_ = 0; }

// Оператор присваивания
DataSignal& DataSignal::operator=(DataSignal const& other){
    if (*this != other){
        property = other.property;
        data_ = other.data_;
    }
    return *this;
}
// Операторы сравнения на равенство
bool DataSignal::operator==(DataSignal const& other) const {
    /* Достаточно сравнить полный путь к файлу, так как отсутствуют set методы
    по отдельности для свойств и сигнала */
    return (property.path_ == other.property.path_ &&
            property.fileName_ == other.property.fileName_);
}
bool DataSignal::operator!=(DataSignal const& other) const { return !(*this == other); }
// Оператор обращения по индексу
double DataSignal::operator[](int index) const { return data_[index]; }

// Пользовательские методы
int DataSignal::size() const { return property.nCount_; } // Длина сигнала
bool DataSignal::isEmpty() const { return (size() == 0); }// Проверка на пустоту сигнала
QVector<double> const& DataSignal::getData() const { return data_; }; // Получение сигнала без свойств
PropertyDataSignal const& DataSignal::getProperty() const { return property; }; // Получение всех свойств
QString DataSignal::getName() const {return property.fileName_; } // Получение имени сигнала

// Чтение текстового файла с временным сигналом
int DataSignal::readDataFile(QString const& path, QString const& fileName){
    QString fileFullPath = path + fileName; // Полный путь к файлу
    QFile file(fileFullPath); // Инициализация файла для чтения
    if (!checkFile(fileFullPath, "read")){ return -1; } // Обработка ошибок
    file.open(QIODevice::ReadOnly | QIODevice::Text); // Открытие файла для чтения
    QTextStream inputStream(&file); // Создание потока чтения
    inputStream.setCodec("cp1251"); // Кодировка CP1251
    // Инициализация свойств класса
    property.path_ = path;                                       // Путь к файлу
    property.fileName_ = fileName;                               // Имя файла
    property.dateAndTime_ = inputStream.readLine();              // Дата и время записи сигнала
    property.measureObject_ = inputStream.readLine();            // Объект измерения
    property.measurePoint_ = inputStream.readLine();             // Точка установки датчика
    property.currentCount_ = inputStream.readLine();             // Текущие отсчеты
    property.temperature_ = inputStream.readLine().toDouble();   // Температура
    property.sensorType_ = inputStream.readLine();               // Тип датчика
    property.physicalFactor_ = inputStream.readLine().toDouble(); // Физический коэффициент
    property.measureUnit_ = inputStream.readLine();              // Единица измерения
    property.scanPeriod_ = inputStream.readLine().toDouble();    // Период опроса датчика
    property.characterisic_ = inputStream.readLine();            // Характеристика
    property.nCount_ = inputStream.readLine().toInt();           // Количество отсчетов
    // Чтение временного сигнала
    data_.clear(); // Очистка сигнала (remove, size -> 0, capacity -> 0)
    data_.resize(property.nCount_); // size() == nCount_
    for (int i = 0; i != property.nCount_; ++i )
        data_[i] = inputStream.readLine().toDouble() * property.physicalFactor_;
    file.close(); // Закрытие файла
    return 0;
}

// Запись временного сигнала
int DataSignal::writeDataFile(QString const& path, QString const& fileName){
    if (isEmpty()) return -1; // Проверка на пустоту записываеомго сигнала
    QString fileFullPath = path + fileName; // Полный путь к файлу
    QFile file(fileFullPath); // Инициализация файла для записи
    if (!checkFile(fileFullPath, "write")){ return -1; } // Обработка ошибок
    file.open(QIODevice::WriteOnly | QIODevice::Text); // Открытие файла для записи
    QTextStream outputStream(&file); // Создание потока для записи
    outputStream.setCodec("cp1251"); // Кодировка CP1251
    outputStream << property.dateAndTime_ << endl;    // Дата и время записи сигнала
    outputStream << property.measureObject_ << endl;  // Объект измерения
    outputStream << property.measurePoint_ << endl;   // Точка установки датчика
    outputStream << property.currentCount_ << endl;   // Текущие отсчеты
    outputStream << property.temperature_ << endl;    // Температура
    outputStream << property.sensorType_ << endl;     // Тип датчика
    outputStream << property.physicalFactor_ << endl;  // Физический коэффициент
    outputStream << property.measureUnit_ << endl;    // Единица измерения
    outputStream << property.scanPeriod_ << endl;     // Период опроса датчика
    outputStream << property.characterisic_ << endl;  // Характеристика
    outputStream << property.nCount_ << endl;         // Количество отсчетов
    for (int i = 0; i != property.nCount_; ++i )
        outputStream << data_[i] / property.physicalFactor_ << endl;
    file.close(); // Закрытие файла
    return 0;
}

