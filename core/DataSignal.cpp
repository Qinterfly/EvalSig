#include <QFile>
#include <QTextStream>
#include <QDebug>
#include "DataSignal.h"

// ---- Временной сигнал ---------------------------------------------------------------------------------------

// Конструктор от пути и имени файла
DataSignal::DataSignal(QString const& path, QString const& fileName, int shift){ readDataFile(path, fileName, shift); }
DataSignal::DataSignal(QVector<double> const& someData, PropertyDataSignal const& someProperty):
    property(someProperty), data_(someData)
{
    if (data_.size() != property.nCount_) // Проверка на согласованность данных
        property.nCount_ = data_.size();
    // Сброс параметров чтения
    property.fileName_ = "";
    property.path_ = "";
}
// Копирующий конструктор по заданной области
DataSignal::DataSignal(DataSignal const& other, int leftInd, int rightInd) : property(other.property) {
    int lastInd = property.nCount_ - 1; // Индекс последнего элемента
    // Обработка обратной индексации
    if (rightInd == -1) rightInd = lastInd;
    // Проверка согласованности границ участка
    if (leftInd > rightInd) return;
    // Проверка на выход за границы
    if (rightInd > lastInd) rightInd = lastInd;
    property.nCount_ = rightInd - leftInd + 1; // Установка числа отсчетов в сигнале
    // Копирование значений сигнала
    data_.resize(property.nCount_); // Аллоцирование памяти
    for (int i = leftInd; i <= rightInd; ++i)
        data_[i - leftInd] = other.data_[i];
}
// Перемещающий конструктор для всего сигнала
DataSignal::DataSignal(DataSignal && tmpOther) noexcept : property(std::move(tmpOther.property)), data_(std::move(tmpOther.data_)) { }
// Перемещающий конструктор для данных сигнала
DataSignal::DataSignal(QVector<double> && someData, PropertyDataSignal && someProperty) noexcept:
    property(std::move(someProperty)), data_(std::move(someData))
{
    if (data_.size() != property.nCount_) // Проверка на согласованность данных
        property.nCount_ = data_.size();
    // Сброс параметров чтения
    property.fileName_ = "";
    property.path_ = "";
}
// Оператор присваивания
DataSignal& DataSignal::operator=(DataSignal const& other){
    if (this != &other){
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

// Получение среднего значения сигнала
double DataSignal::mean() const { return meanVec(data_); }

// Нормализация сигнала
void DataSignal::normalize(NormalizeOption option) {
    normalizeVec(data_, option);
}

// Чтение текстового файла с временным сигналом
int DataSignal::readDataFile(QString const& path, QString const& fileName, int shift){
    QString fileFullPath = path + fileName; // Полный путь к файлу
    QFile file(fileFullPath); // Инициализация файла для чтения
    if ( !checkFile(fileFullPath, "read") ){ return -1; } // Обработка ошибок
    file.open(QIODevice::ReadOnly | QIODevice::Text); // Открытие файла для чтения
    QTextStream inputStream(&file); // Создание потока чтения
    inputStream.setCodec("cp1251"); // Кодировка CP1251
    // Инициализация свойств класса
    property.path_ = path;                                        // Путь к файлу
    property.fileName_ = fileName;                                // Имя файла
    property.dateAndTime_ = inputStream.readLine();               // Дата и время записи сигнала
    if ( !property.dateAndTime_.contains(":") ) return -1;        // Проверка формата файла
    property.measureObject_ = inputStream.readLine();             // Объект измерения
    property.measurePoint_ = inputStream.readLine();              // Точка установки датчика
    property.currentCount_ = inputStream.readLine();              // Текущие отсчеты
    property.temperature_ = inputStream.readLine().toDouble();    // Температура
    property.sensorType_ = inputStream.readLine();                // Тип датчика
    property.physicalFactor_ = inputStream.readLine().toDouble(); // Физический коэффициент
    property.measureUnit_ = inputStream.readLine();               // Единица измерения
    property.scanPeriod_ = inputStream.readLine().toInt();        // Период опроса датчика
    property.characteristic_ = inputStream.readLine();            // Характеристика
    property.nCount_ = inputStream.readLine().toInt();            // Количество отсчетов
    property.isSpectrum = property.fileName_.contains("Спектр") || property.characteristic_.contains("Спектр");  // Является ли сигнал спектром       
    // Чтение временного сигнала
    data_.clear(); // Очистка сигнала (remove, size -> 0, capacity /-> 0)
    data_.resize(property.nCount_); // size() == nCount_
    // Предобработка смещения
    int nRead = property.nCount_;
    int iInsert = 0;
    double tValue = 0;
    // Смещение влево
    if (shift < 0) {
        nRead += shift;
        tValue = shift;
        while (tValue < 0 && !inputStream.atEnd()){
            inputStream.readLine();
            ++tValue;
        }
    } else if (shift > 0) { // Смещение вправо
        iInsert = shift;
        nRead -= shift;
    }
    if (nRead <= 0) return 1;
    // Чтение данных
    for (int i = 0; i != nRead; ++i )
        data_[iInsert + i] = inputStream.readLine().toDouble() * property.physicalFactor_;
    // Копирование краевых значений
    if (shift != 0){
        tValue = data_[iInsert];
        // Слева
        for (int j = 0; j != iInsert; ++j)
            data_[j] = tValue;
        // Справа
        tValue = data_[iInsert + nRead - 1];
        for (int j = nRead; j != property.nCount_; ++j)
            data_[iInsert + j] = tValue;
    }
    file.close(); // Закрытие файла
    return 0;
}

// Запись временного сигнала на выбранном участке
int DataSignal::writeDataFile(QString const& path, QString const& fileName, int leftInd, int rightInd) const {
    if (isEmpty()) return -1; // Проверка на пустоту записываеомго сигнала
    if (rightInd == -1) rightInd = property.nCount_ - 1; // Обработка индекса последнего значения
    if (leftInd > rightInd) return -1; // Проверка на корректность области сохранения
    QString fileFullPath = path + fileName; // Полный путь к файлу
    QFile file(fileFullPath); // Инициализация файла для записи
    if (!checkFile(fileFullPath, "write")){ return -1; } // Обработка ошибок
    file.open(QIODevice::WriteOnly | QIODevice::Text); // Открытие файла для записи
    QTextStream outputStream(&file); // Создание потока для записи
    outputStream.setCodec("cp1251"); // Кодировка CP1251
    outputStream << property.dateAndTime_ << endl;     // Дата и время записи сигнала
    outputStream << property.measureObject_ << endl;   // Объект измерения
    outputStream << property.measurePoint_ << endl;    // Точка установки датчика
    outputStream << property.currentCount_ << endl;    // Текущие отсчеты
    outputStream << property.temperature_ << endl;     // Температура
    outputStream << property.sensorType_ << endl;      // Тип датчика
    outputStream << property.physicalFactor_ << endl;  // Физический коэффициент
    outputStream << property.measureUnit_ << endl;     // Единица измерения
    outputStream << property.scanPeriod_ << endl;      // Период опроса датчика
    outputStream << property.characteristic_ << endl;   // Характеристика
    outputStream << rightInd - leftInd + 1 << endl;    // Количество отсчетов
    for (int i = leftInd; i <= rightInd; ++i )
        outputStream << data_[i] / property.physicalFactor_ << endl;
    file.close(); // Закрытие файла
    return 0;
}

// Изменение параметров сигнала
void DataSignal::setFileName(QString const& fileName) { property.fileName_ = fileName; }                         // Имя файла
void DataSignal::setDateAndTime(QString const& dateAndTime) { property.dateAndTime_ = dateAndTime; }             // Время записи
void DataSignal::setMeasureObject(QString const& measureObject) { property.measureObject_ = measureObject; }     // Объект измерения
void DataSignal::setMeasurePoint(QString const& measurePoint) { property.measurePoint_ = measurePoint; };        // Точка установки датчика
void DataSignal::setTemperature(double temperature) { property.temperature_ = temperature; }                     // Температура
void DataSignal::setSensorType(QString const& sensorType) { property.sensorType_ = sensorType; }                 // Тип датчика
void DataSignal::setCharacteristic(QString const& characteristic) { property.characteristic_ = characteristic; } // Характеристика
void DataSignal::setMeasureUnit(QString const& measureUnit) { property.measureUnit_ = measureUnit; }             // Единицы измерения
void DataSignal::setScanPeriod(int scanPeriod){ property.scanPeriod_ = scanPeriod; }                             // Период опроса датчика
// Физический коэффициент
void DataSignal::setPhysicalFactor(double physicalFactor){
    for (double & elem : data_)
        elem = elem / property.physicalFactor_ * physicalFactor;
    property.physicalFactor_ = physicalFactor;
}

// ---- Вспомогательные функции --------------------------------------------------------------------------------

// Поиск минимума-максимума в векторе
QPair<double, double> minMaxVec(QVector<double> const& vec, int leftInd, int rightInd){
    if (rightInd == -1) rightInd = vec.size() - 1; // Обработка обратной индексации
    auto res = std::minmax_element(vec.begin() + leftInd, vec.begin() + rightInd + 1);
    return {*res.first, *res.second};
}

// Поиск максимума в векторе
double maxVec(QVector<double> const& vec, int leftInd, int rightInd){
    return *std::max(vec.begin() + leftInd, vec.begin() + rightInd + 1);
}

// Вычисление среднего для вектора
double meanVec(QVector<double> const& vec, int leftInd, int rightInd){
    if (rightInd == -1) rightInd = vec.size() - 1; // Обработка обратной индексации
    double sum = 0;
    for (int i = leftInd; i <= rightInd; ++i)
        sum += vec[i];
    return ( sum / (rightInd - leftInd + 1) );
}

// Нормализация вектора
void normalizeVec(QVector<double> & vec, NormalizeOption option, int leftInd, int rightInd){
    if (rightInd == -1) rightInd = vec.size() - 1; // Обработка обратной индексации
    double zeroLineVal = 0;
    switch (option){
    case FIRST: // По первому значению
        zeroLineVal = vec[0];
        break;
    case MEAN: // По среднему значению
        zeroLineVal = meanVec(vec, leftInd, rightInd);
        break;
    }
    for (int i = leftInd; i <= rightInd; ++i)
        vec[i] -= zeroLineVal;
}

// -------------------------------------------------------------------------------------------------------------

