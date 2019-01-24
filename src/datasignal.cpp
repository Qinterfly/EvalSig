#include "datasignal.h"

// Проверка файла на существование и на режим записи / чтения
bool checkFile(QString const& fileFullPath, QString const& mode) {
    // mode == Read -- чтение
    // mode == Write -- запись

    // Проверка существования файла
    if ( !QFileInfo::exists(fileFullPath) || !QFileInfo(fileFullPath).isFile() ){
        qDebug() << "Файл:" << fileFullPath << "не найден";
        return 0;
    }
    // Проверка на возможность чтения
    if ( mode == "read" ){
        if ( !QFileInfo(fileFullPath).isReadable() ){
            qDebug() << "Файл:" << fileFullPath << "не доступен для чтения";
            return 0;
        }
        if ( QFileInfo(fileFullPath).size() == 0 ){ // Проверка на пустоту файла
            qDebug() << "Файл:" << fileFullPath << "не содержит данных";
            return 0;
        }
    }
    // Проверка на возможность записи
    if ( mode == "write" )
        if ( !QFileInfo(fileFullPath).isWritable() ){
            qDebug() << "Файл:" << fileFullPath << "не доступен для записи";
            return 0;
        }
    return 1;
}

// Конструктор от имени файла
DataSignal::DataSignal(QString const& path, QString const& fileName){ readDataFile(path, fileName); }

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
bool DataSignal::operator!=(DataSignal const& other) const { return (*this == other); }

// Пользовательские методы
int DataSignal::size() const { return property.nCount_; } // Длина сигнала
QVector<double> DataSignal::getData() const { return data_; }; // Получение сигнала без свойств
PropertyDataSignal DataSignal::getProperty() const { return property; }; // Получение свойств

// Чтение текстового файла с временным сигналом
void DataSignal::readDataFile(QString const& path, QString const& fileName){
    QString fileFullPath = path + fileName; // Полный путь к файлу
    QFile file(fileFullPath); // Инициализация файла для чтения
    if (!checkFile(fileFullPath, "read")){ return; } // Обработка ошибок
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
    property.physicalCoeff_ = inputStream.readLine().toDouble(); // Физический коэффициент
    property.measureUnit_ = inputStream.readLine();              // Единица измерения
    property.scanPeriod_ = inputStream.readLine().toDouble();    // Период опроса датчика
    property.characterisic_ = inputStream.readLine();            // Характеристика
    property.nCount_ = inputStream.readLine().toInt();           // Количество отсчетов
    // Чтение временного сигнала
    data_.clear(); // Очистка сигнала (remove, size -> 0, capacity -> 0)
    data_.resize(property.nCount_); // size() == nCount_
    for (int i = 0; i != property.nCount_; ++i )
        data_[i] = inputStream.readLine().toDouble();
}

