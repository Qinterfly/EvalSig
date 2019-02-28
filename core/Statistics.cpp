#include <QDebug>
#include <QDir>
#include <QtMath>
#include "Statistics.h"

// ---- Статистики группы сигналов -----------------------------------------------------------------------------

// Конструктор Statistics
Statistics::Statistics(QVector<DataSignal> & vecDataSignal, int widthTimeWindow, int shiftTimeWindow)
    : pVecDataSignal(&vecDataSignal), nSize_(pVecDataSignal->size()),
      minSizeSignals_(calcMinSizeSignals()),
      windowProperty(widthTimeWindow, shiftTimeWindow, minSizeSignals_)
{
    allocateAllFields(0, nSize_); // Выделение памяти для хранения полей
    fullCompute(); // Полный расчет матрицы характеристик
}

// Интерфейс пользователя
int Statistics::size() const { return nSize_; } // Текущий размер матрицы статистик
bool Statistics::isEmpty() const { return size() == 0; } // Проверка на пустоту
int Statistics::minSizeSignals() const { return minSizeSignals_; }; // Минимальная длина сигнала из группы
int Statistics::getNumberOfWindows() const { return windowProperty.nWindows_; } // Получить число временных окон (без учета среднего)
ArrayRegressionParams const& Statistics::getRegressionParams() const { return regressionParams_; } // Получение регрессионных параметров
ArrayStatCharacters const& Statistics::getDistanceScatter() const { return distanceScatter_; } // Получение дистанций рассеяния
ArrayStatCharacters const& Statistics::getSimilarityCoeffs() const { return similarityCoeffs_; } // Получение коэффициентов подобия сигналов
ArrayStatCharacters const& Statistics::getAmplitudeScatter() const { return amplitudeScatter_; } // Получение амплитуд рассеяния
ArrayStatCharacters const& Statistics::getNoiseCoeffs() const { return noiseCoeffs_; } // Получение коэффициентов шума

    // Добавление сигнала
bool Statistics::addSignal(DataSignal const& dataSignal){
    int sizeSignal = dataSignal.size(); // Длина сигнала
    pVecDataSignal->push_back(dataSignal);   // Добавление объекта в вектор сигналов
    // Оценка необходимости полного пересчета матрицы
    if (sizeSignal >= minSizeSignals_ && minSizeSignals_ != 0){
        allocateAllFields(nSize_, nSize_ + 1); // Инциализация дополнительных полей
        ++nSize_; // Увеличение размера матрицы статистик
        partialCompute(); // Вызов метода частичного пересчета
    }
    else { // Полный пересчет
        minSizeSignals_ = sizeSignal; // Запись новой наименьшой длины сигнала
        windowProperty.calcWindowParams(minSizeSignals_); // Пересчет параметров окна
        allocateAllFields(0, nSize_ + 1); // Инциализация дополнительных полей
        ++nSize_; // Увеличение размера матрицы статистик
        fullCompute(); // Вызов метода полного пересчета
    }
    return 0;
}
    // Удаление сигнала
bool Statistics::removeSignal(int deleteInd){
    if (isEmpty()) { qDebug() << "Объект статистик пуст"; return 1; } // Проверка на пустоту
    if (deleteInd < 0 || deleteInd > nSize_ - 1){ // Проверка на возможность удаления
        qDebug() << "Попытка удаления несуществующего элемента";
        return 1;
    }
    pVecDataSignal->remove(deleteInd); // Удаление объекта из вектора сигналов
    removeAllFields(deleteInd); // Удаление статистик, связанных с объектов
    --nSize_; // Уменьшение размера матрицы
    int tempMinSizeSignals = calcMinSizeSignals(); // Получение нового минимального размера группы сигналов
    if (tempMinSizeSignals != minSizeSignals_){ // Если после удаление минимальный размер сигналов изменился
        minSizeSignals_ = tempMinSizeSignals; // Запись нового размера
        windowProperty.calcWindowParams(minSizeSignals_); // Пересчет параметров окна
        allocateAllFields(0, nSize_); // Выделение памяти для хранения полей
        fullCompute(); // Вызов метода полного пересчета
    }
    return 0;
}
    // Сохранение всех статистик
int Statistics::writeAllStatistics(QString const& dirName){
    if (isEmpty()) return -1; // Проверка на пустоту статистик
    QDir dir(dirName); // Инициализация директории c добавлением разделителя
    if (!dir.exists()) // Проверка существования директории
        dir.mkpath(".");
    windowProperty.writeWindowParams(dirName, "Параметры временного окна.txt"); // Запись параметров расчета
    writeSignalList(dirName, "Список сигналов.txt"); // Запись списка сигналов
    // Создание поддиректорий для каждой из статистик
    QVector<QString> vecStatName = {"Угловые коэффициенты", "Дистанции рассеяния",
                                "Коэффициенты подобия", "Амплитуды рассеяния", "Коэффициенты шума"}; // Имена статистик
    // Создание пути для каждой из статистик
    for (QString & statName : vecStatName)
        dir.mkdir(statName);
    // Сохранение статистик
    int exitStatus = 0; // Код возврата
    exitStatus += writeStatistic(regressionParams_, dirName, vecStatName[0]); // Угловые коэффициенты
    exitStatus += writeStatistic(distanceScatter_, dirName, vecStatName[1]);  // Дистанция рассеяния
    exitStatus += writeStatistic(similarityCoeffs_, dirName, vecStatName[2]); // Коэффициенты подобия
    exitStatus += writeStatistic(amplitudeScatter_, dirName, vecStatName[3]); // Амплитуды рассеяния
    exitStatus += writeStatistic(noiseCoeffs_, dirName, vecStatName[4]); // Коэффициенты шума
    return exitStatus;
}

// Сохранение выбранной статистики
 template<typename T>
int Statistics::writeStatistic(T const& stat, QString const& dirName, QString const& statName){
    QString path = dirName + statName + QDir::separator(); // Полный путь до статистики
    QVector<double> tData; // Контейнер статистик для выбранной пары сигналов
    int exitStatus = 0; // Код возврата
    for (int i = 0; i != nSize_; ++i){ // По числу сигналов
        // Добавление статистики по всем окнам в контейнер
        PropertyDataSignal tProperty = (*pVecDataSignal)[i].getProperty(); // Получение свойств первого сигнала
        for (int j = 0; j != nSize_; ++j){
            tData = getWindowStatisticData(stat, i, j); // Получение временных данных
            tProperty.nCount_ = tData.size(); // Запись длины сигнала
            tProperty.currentCount_ = statName; // Имя статистики
            DataSignal tDataSignal(tData, tProperty); // Создание временного сигнала
            QString fileName = QString::number(i + 1) + "-" + QString::number(j + 1) + ".txt"; // Определение имени временного сигнала
            exitStatus += tDataSignal.writeDataFile(path, fileName); // Запись сигнала без конвертации
        }
    }
    return exitStatus;
}

// Запись списка сигналов
int Statistics::writeSignalList(QString const& path, QString const& fileName) {
    QString fileFullPath = path + fileName; // Полный путь к файлу
    QFile file(fileFullPath); // Инициализация файла для записи
    if (!checkFile(fileFullPath, "write")){ return -1; } // Обработка ошибок
    file.open(QIODevice::WriteOnly | QIODevice::Text); // Открытие файла для записи
    QTextStream outputStream(&file); // Создание потока для записи
    outputStream.setCodec("cp1251"); // Кодировка CP1251
    int iSignal = 0;
    for (DataSignal const& signal : *pVecDataSignal){
        outputStream << QString::number(++iSignal) << ": " << signal.getName() << endl;
    }
    file.close(); // Закрытие файла
    return 0;
}

    // Изменение свойств окна
void Statistics::setWindowProperty(int widthTimeWindow, int shiftTimeWindow){
    // Проверка необходимости изменения
    if (windowProperty.width_ == widthTimeWindow && windowProperty.shiftWindow_ == shiftTimeWindow)
        return;
    // Запись новых параметров
    windowProperty.width_ = widthTimeWindow;
    windowProperty.shiftWindow_ = shiftTimeWindow;
    windowProperty.calcWindowParams(minSizeSignals_);
    allocateAllFields(0, nSize_); // Выделение памяти для хранения полей
    fullCompute(); // Полный пересчет
}

// Выделение памяти для полей структуры
    // При расширении объекта
template<typename T>
void Statistics::allocateField(T& field, int beginColInd, int fullSize){
    field.resize(fullSize); // Строки
    for (int i = 0; i != fullSize; ++i){ // Столбцы
        field[i].resize(fullSize);
        for (int j = beginColInd; j != fullSize; ++j)
            field[i][j].resize(windowProperty.nWindows_ + 1); // (!) Выделяем под заданное число окон + под среднее окно
    }
    // В случае расширения матрицы одним сигналом дополнительно
    // инициализируем последнюю строку, за исключением диагонального элемента
    if (beginColInd == fullSize - 1)
        for (int j = 0; j != fullSize - 1; ++j)
            field[fullSize - 1][j].resize(windowProperty.nWindows_ + 1); // (!) Выделяем под заданное число окон + под среднее окно
}
    // При сжатии объекта
template<typename T>
void Statistics::removeField(T& field, int deleteInd){
    field.remove(deleteInd); // Удаление строки по индексу
    for (int i = 0; i != nSize_ - 1; ++i)
        field[i].remove(deleteInd); // Удаление столбца по индексу в оставшихся строках
}

// Методы-обертки для выделения памяти для всех полей
    // При расширении для всех полей
void Statistics::allocateAllFields(int beginColInd, int fullSize){
    allocateField(regressionParams_, beginColInd, fullSize);   // Параметры линейной регрессии
    allocateField(distanceScatter_, beginColInd, fullSize);    // Дистанция рассеяния
    allocateField(similarityCoeffs_, beginColInd, fullSize);   // Коэффициенты подобия сигналов
    allocateField(amplitudeScatter_, beginColInd, fullSize);   // Амплитуда рассеяния
    allocateField(noiseCoeffs_, beginColInd, fullSize);        // Коэффициенты шума
}

    // При сжатии для всех полей
void Statistics::removeAllFields(int deleteInd){
    removeField(regressionParams_, deleteInd);   // Параметры линейной регрессии
    removeField(distanceScatter_, deleteInd);    // Дистанция рассеяния
    removeField(similarityCoeffs_, deleteInd);   // Коэффициенты подобия сигналов
    removeField(amplitudeScatter_, deleteInd);   // Амплитуда рассеяния
    removeField(noiseCoeffs_, deleteInd);        // Коэффициенты шума
}

// Нахождение минимального размера сигнала из группы
int Statistics::calcMinSizeSignals(){
    if (pVecDataSignal->isEmpty()) return 0; // Проверка на пустоту
    QVector<DataSignal>::iterator iter = pVecDataSignal->begin();
    int tempMinSize = iter->size(); ++iter;
    for ( ; iter != pVecDataSignal->end(); ++iter)
        if (iter->size() < tempMinSize) tempMinSize = iter->size();
    return tempMinSize;
}

// Полный расчет статистик
void Statistics::fullCompute(){
    // Расчет регрессионных параметров, дистанций и амплитуд рассеяния
    for (int i = 0; i != nSize_; ++i)
        for (int j = 0; j != nSize_; ++j)
            calcDistanceAmplitudeRegression(i, j);
    // Расчет коэффициентов подобия
    for (int i = 0; i != nSize_; ++i) // По всем сигналам
        for (int j = 0; j != nSize_; ++j)
            calcSimilarity(i, j);
}

// Частичный пересчет статистик (при добавлении одного сигнала)
void Statistics::partialCompute(){
    // Расчет регрессионных параметров, дистанций и амплитуд рассеяния
        // По последнему столбцу
    for (int i = 0; i != nSize_; ++i)
        calcDistanceAmplitudeRegression(i, nSize_ - 1);
        // По последней строке, за исключением диагонального элемента
    for (int j = 0; j != nSize_ - 1; ++j)
        calcDistanceAmplitudeRegression(nSize_ - 1, j);
    // Расчет коэффициентов подобия [циклы объединить нельзя]
        // По последнему столбцу
    for (int i = 0; i != nSize_; ++i)
        calcSimilarity(i, nSize_ - 1);
    // По последней строке, за исключением диагонального элемента
    for (int j = 0; j != nSize_ - 1; ++j)
        calcSimilarity(nSize_ - 1, j);
}

// Тело цикла пересчета для дистанций, амплитуд и регрессионных параметров
void Statistics::calcDistanceAmplitudeRegression(int i, int j){
    int currWindow = 0; // Номер текущего окна
    // Параметры среднего окна
    QPair<double, double> meanRegressionParams = {0, 0}; // Средние регрессионные параметры
    double meanDistanceScatter = 0; // Средняя дистанция рассеяния
    double meanAmplitudeScatter = 0; // Средняя амплитуда рассеяния
    // По всем окнам
    for (int s = 0; s < minSizeSignals_; ){ // Пока левая граница не достигнет конца сигнала
        int currRightBound = windowProperty.width_;
        if (currRightBound + s > minSizeSignals_) // Проверка правой границы
            currRightBound = minSizeSignals_ - s;
        // Нахождение средних значений
        double meanX = 0, meanY = 0;
        for (int k = 0; k != currRightBound; ++k){
            meanX += (*pVecDataSignal)[i][s + k];
            meanY += (*pVecDataSignal)[j][s + k];
        }
        meanX /= currRightBound; // Нормировка к реальной ширине окна
        meanY /= currRightBound;
        // Нахождение параметров линейной регрессии
        double numeratorA = 0, denominatorA = 0; // Числитель и знаменатель углового коэффициента
        for (int k = 0; k != currRightBound; ++k){
            numeratorA += ( (*pVecDataSignal)[i][s + k] - meanX ) * ( (*pVecDataSignal)[j][s + k] - meanY );
            denominatorA += qPow( (*pVecDataSignal)[i][s + k] - meanX, 2 );
        }
        if (numeratorA == 0 && denominatorA == 0)
            regressionParams_[i][j][currWindow].first = 0; // Угловой коэффициент
        else
            regressionParams_[i][j][currWindow].first = numeratorA / denominatorA; // Угловой коэффициент
        regressionParams_[i][j][currWindow].second = meanY - regressionParams_[i][j][currWindow].first * meanX; // Смещение прямой
        double alpha = qAtan(regressionParams_[i][j][currWindow].first); // Угол наклона прямой
        double tSumDistance = 0; // Подсумма дистанции рассеяния
        double tXSignal = 0, tYSignal = 0; // Для амплитуды рассеяния
        for (int k = 0; k != currRightBound; ++k){
            // Вычисление регрессионной функции
            double tLinearRegressionFun = regressionParams_[i][j][currWindow].first * (*pVecDataSignal)[i][s + k] + regressionParams_[i][j][currWindow].second;
            // Вычисление подсуммы дистанции рассеяния
            tSumDistance += 1. / minSizeSignals_* qFabs( (*pVecDataSignal)[j][s + k] - tLinearRegressionFun ) * qCos(alpha);
            // Для амплитуды рассеяния
            tXSignal += qAbs( (*pVecDataSignal)[i][s + k] - meanX );
            tYSignal += qAbs( (*pVecDataSignal)[j][s + k] - meanY );
        }
        distanceScatter_[i][j][currWindow] = tSumDistance; // Дистанция рассеяния
        if (tXSignal == 0 && tYSignal == 0)
            amplitudeScatter_[i][j][currWindow] = 0;
        else
            amplitudeScatter_[i][j][currWindow] = tSumDistance * minSizeSignals_ / qSqrt(qPow(tXSignal, 2) + qPow(tYSignal, 2)); // Амплитуда рассеяния
        // Суммирование значений для среднего окна
        meanRegressionParams.first += regressionParams_[i][j][currWindow].first; // Угловых коэффициентов
        meanRegressionParams.second += regressionParams_[i][j][currWindow].second; // Смещения прямых
        meanDistanceScatter += distanceScatter_[i][j][currWindow]; // Дистанций рассеяния
        meanAmplitudeScatter += amplitudeScatter_[i][j][currWindow]; // Амплитуд рассеяния
        // Сдвиг
        s += windowProperty.shiftWindow_; // Сдвиг левой границы окна
        currWindow += 1; // Приращение счетчика окон
    }
    // Нормирование и запись сумм (в nWindows + 1 окно)
    regressionParams_[i][j][currWindow].first = meanRegressionParams.first / currWindow; // Угловых коэффициентов прямых
    regressionParams_[i][j][currWindow].second = meanRegressionParams.second / currWindow; // Смещения прямых
    distanceScatter_[i][j][currWindow] = meanDistanceScatter / currWindow; // Дистанций рассеяния
    amplitudeScatter_[i][j][currWindow] = meanAmplitudeScatter / currWindow; // Амплитуд рассеяния
}

// Тело цикла для расчета коэффициентов подобия
void Statistics::calcSimilarity(int i, int j){
    int currWindow = 0; // Номер текущего окна
    // Параметры среднего окна
    double meanSimilarityCoeffs = 0; // Средний коэффициент подобия
    double meanNoiseCoeffs = 0; // Средние коэффициенты шума
    // По всем окнам
    for (int s = 0; s < minSizeSignals_; ){ // Пока левая граница не достигнет конца сигнала
        int currRightBound = windowProperty.width_;;
        if (currRightBound + s > minSizeSignals_) // Проверка правой границы
            currRightBound = minSizeSignals_ - s;
        similarityCoeffs_[i][j][currWindow] = qSqrt(regressionParams_[i][j][currWindow].first * regressionParams_[j][i][currWindow].first);
        noiseCoeffs_[i][j][currWindow] = qSqrt(amplitudeScatter_[i][j][currWindow] * amplitudeScatter_[j][i][currWindow]);
        // Суммирование значений для среднего окна
        meanSimilarityCoeffs += similarityCoeffs_[i][j][currWindow]; // Коэффициент подобия
        meanNoiseCoeffs += noiseCoeffs_[i][j][currWindow]; // Коэффициент шума
        // Сдвиг
        s += windowProperty.shiftWindow_; // Сдвиг левой границы окна
        currWindow += 1; // Приращение счетчика окон
    }
    // Нормирование и запись сумм (в nWindows + 1 окно)
    similarityCoeffs_[i][j][currWindow] = meanSimilarityCoeffs / currWindow; // Коэффициент подобия
    noiseCoeffs_[i][j][currWindow] = meanNoiseCoeffs / currWindow; // Коэффициент шума
}

// ---- Вспомогательные функции --------------------------------------------------------------------------------

// Получение оконного распределения статистики
    // ArrayRegressionParams
QVector<double> Statistics::getWindowStatisticData(ArrayRegressionParams const& stat, int i, int j){
    QVector<double> tData(windowProperty.nWindows_); // Вектор данных для хранения статистик
    for (int s = 0; s != windowProperty.nWindows_; ++s){ // По всем окнам, за исключением среднего
        tData[s] = stat[i][j][s].first; // Угловые коэффициенты
    }
    return tData;
}
    // ArrayStatCharacters
QVector<double> Statistics::getWindowStatisticData(ArrayStatCharacters const& stat, int i, int j){
    QVector<double> tData(windowProperty.nWindows_); // Вектор данных для хранения статистик
    for (int s = 0; s != windowProperty.nWindows_; ++s){ // По всем окнам, за исключением среднего
        tData[s] = stat[i][j][s];
    }
    return tData;
}

// -------------------------------------------------------------------------------------------------------------
