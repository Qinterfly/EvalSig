#include <QDebug>
#include <QDir>
#include <QtMath>
#include "Statistics.h"
#include "xlsxdocument.h"
#include "xlsxformat.h"


// ---- Статистики группы сигналов -----------------------------------------------------------------------------

// Объявление вспомогательных функций
    // Перевод числа в строку
QString numberToString(int const& value);
QString numberToString(double const& value, QString const& delimiter = ",");

// Конструктор Statistics
Statistics::Statistics(QVector<DataSignal> & vecDataSignal, int widthTimeWindow, int shiftTimeWindow, int leftEstimationBoundary, int rightEstimationBoundary)
    : pVecDataSignal(&vecDataSignal), nSize_(pVecDataSignal->size()),
      minSizeSignals_(calcMinSizeSignals()),
      estimationBoundaries_(leftEstimationBoundary, rightEstimationBoundary),
      windowProperty(widthTimeWindow, shiftTimeWindow, estimationBoundaries_, minSizeSignals_)
{
    allocateAllFields(0, nSize_); // Выделение памяти для хранения полей
    fullCompute(); // Полный расчет матрицы характеристик
    allocateAllMetrics(); // Выделение памяти для всех метрик
    calcAllMetrics();     // Расчет всех метрик
}

// Интерфейс пользователя
    // Добавление сигнала
int Statistics::addSignal(DataSignal const& dataSignal){
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
        checkEstimationBoundaries(); // Проверка расчетных границ
        windowProperty.calcWindowParams(estimationBoundaries_, minSizeSignals_); // Пересчет параметров окна
        allocateAllFields(0, nSize_ + 1); // Инциализация дополнительных полей
        ++nSize_; // Увеличение размера матрицы статистик
        fullCompute(); // Вызов метода полного пересчета
    }
    allocateAllMetrics(); // Выделение памяти для всех метрик
    calcMetric(nSize_ - 1); // Расчет метрик
    return 0;
}
    // Удаление сигнала
int Statistics::removeSignal(int deleteInd){
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
        checkEstimationBoundaries(); // Проверка расчетных границ
        windowProperty.calcWindowParams(estimationBoundaries_, minSizeSignals_); // Пересчет параметров окна
        allocateAllFields(0, nSize_); // Выделение памяти для хранения полей
        fullCompute(); // Вызов метода полного пересчета
    }
    removeAllMetrics(deleteInd); // Удаление метрик сигнала по индексу
    return 0;
}
    // Сохранение всех статистик
int Statistics::writeAllStatistics(QString const& dirName) const {
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
    // Сохранение статистик по всем окнам
    int exitStatus = 0; // Код возврата
    exitStatus += writeStatistic(regressionParams_, dirName, vecStatName[0]); // Угловые коэффициенты
    exitStatus += writeStatistic(distanceScatter_, dirName, vecStatName[1]);  // Дистанция рассеяния
    exitStatus += writeStatistic(similarityCoeffs_, dirName, vecStatName[2]); // Коэффициенты подобия
    exitStatus += writeStatistic(amplitudeScatter_, dirName, vecStatName[3]); // Амплитуды рассеяния
    exitStatus += writeStatistic(noiseCoeffs_, dirName, vecStatName[4]); // Коэффициенты шума
    // Сохранение средних значений всех статистик
    exitStatus += writeMeanStatistics(dirName, "Средние значения статистик");
    return exitStatus;
}

    // Сохранение выбранной статистики
template<typename T>
int Statistics::writeStatistic(T const& stat, QString const& dirName, QString const& statName) const {
    QString path = dirName + statName + QDir::separator(); // Полный путь до статистики
    QVector<double> tData; // Контейнер статистик для выбранной пары сигналов
    int exitStatus = 0; // Код возврата
    for (int i = 0; i != nSize_; ++i){ // По числу сигналов
        // Добавление статистики по всем окнам в контейнер
        PropertyDataSignal tProperty = (*pVecDataSignal)[i].getProperty(); // Получение свойств первого сигнала
        tProperty.physicalFactor_ = 1; // Безразмерные коэффициенты
        for (int j = 0; j != nSize_; ++j){
            tData = getWindowStatisticData(stat, i, j); // Получение временных данных
            tProperty.nCount_ = tData.size(); // Запись длины сигнала
            tProperty.measurePoint_ = statName; // Имя статистики
            DataSignal tDataSignal(tData, tProperty); // Создание временного сигнала
            QString fileName = QString::number(i + 1) + "-" + QString::number(j + 1) + ".txt"; // Определение имени временного сигнала
            exitStatus += tDataSignal.writeDataFile(path, fileName); // Запись сигнала без конвертации
        }
    }
    return exitStatus;
}

    // Сохранение средних значений всех статистик
int Statistics::writeMeanStatistics(QString const& dirName, QString const& fileName) const{
    QString fullFilePath = dirName + fileName + ".xlsx"; // Полный путь к файлу
    int exitStatus = 0; // Код возврата
    QXlsx::Document xlsxDocument; // Создание документа расширения .xlsx
    // Установка формата документа
    QXlsx::Format tableFormat;
    tableFormat.setHorizontalAlignment(QXlsx::Format::AlignHCenter); // Выравнивание по горизонтали
    tableFormat.setVerticalAlignment(QXlsx::Format::AlignVCenter); // Выравнивание по вертикали
    xlsxDocument.setColumnWidth(1, 6, 11); // Ширина рабочих столбцов
    // Заполнение шапки документа
        // Расчетный диапазон
    QString tempString = "Границы участка: " + numberToString(estimationBoundaries_.first) + " - " + numberToString(estimationBoundaries_.second);
    xlsxDocument.write("A1", tempString);
        // Длина участка
    tempString = "Длина участка: " + numberToString(estimationBoundaries_.second - estimationBoundaries_.first + 1);
    xlsxDocument.write("A2", tempString);
        // Заголовки таблицы
    xlsxDocument.write("A4", "Сигнал", tableFormat); xlsxDocument.write("B4", "Угловые", tableFormat);
    xlsxDocument.write("C4", "Дистанции", tableFormat); xlsxDocument.write("D4", "Подобия", tableFormat); // ... в той же строке
    xlsxDocument.write("E4", "Амплитуды", tableFormat); xlsxDocument.write("F4", "Шума", tableFormat);    // ... в той же строке
    int lastLine = 4; // Номер последней линии шапки
    // Запись расчетных данных
    int nMeanWindow = windowProperty.nWindows_; // Номер среднего окна
    for (int i = 0; i != nSize_; ++i){
        for (int j = 0; j != nSize_; ++j){
            ++lastLine; // Приращение счетчика текущей линии
            xlsxDocument.write("A" + numberToString(lastLine), numberToString(i + 1) + " - " + numberToString(j + 1), tableFormat);     // Сигнал
            xlsxDocument.write("B" + numberToString(lastLine), numberToString(regressionParams_[i][j][nMeanWindow].first), tableFormat); // Угловые
            xlsxDocument.write("C" + numberToString(lastLine), numberToString(distanceScatter_[i][j][nMeanWindow]), tableFormat);        // Дистанции
            xlsxDocument.write("D" + numberToString(lastLine), numberToString(similarityCoeffs_[i][j][nMeanWindow]), tableFormat);       // Подобия
            xlsxDocument.write("E" + numberToString(lastLine), numberToString(amplitudeScatter_[i][j][nMeanWindow]), tableFormat);       // Амплитуды
            xlsxDocument.write("F" + numberToString(lastLine), numberToString(noiseCoeffs_[i][j][nMeanWindow]), tableFormat);            // Шума
        }
    }
    exitStatus = !xlsxDocument.saveAs(fullFilePath); // Сохранение документа
    return exitStatus;
}


    // Запись списка сигналов
int Statistics::writeSignalList(QString const& path, QString const& fileName) const {
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
    windowProperty.calcWindowParams(estimationBoundaries_, minSizeSignals_);
    allocateAllFields(0, nSize_); // Выделение памяти для хранения полей
    fullCompute(); // Полный пересчет
}

    // Выставление расчетных границ
void Statistics::setEstimationBoundaries(int leftBound, int rightBound){
    // Проверка необходимости изменения
    if (leftBound == estimationBoundaries_.first && rightBound == estimationBoundaries_.second)
        return;
    estimationBoundaries_ = {leftBound, rightBound}; // Установка границ
    windowProperty.calcWindowParams(estimationBoundaries_, minSizeSignals_); // Расчет новых параметров временного окна
    allocateAllFields(0, nSize_); // Выделение памяти для хранения полей
    fullCompute(); // Полный пересчет
    calcAllMetrics(); // Пересчет всех метрик
}

    // Пересчет
void Statistics::recalculate(){ fullCompute(); }

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

// Проверка корректности расчетных границ
void Statistics::checkEstimationBoundaries(){
    // Проверка левой границы
    if (!pVecDataSignal->isEmpty() && estimationBoundaries_.first >= minSizeSignals_)
        estimationBoundaries_.first = 1;
    // Проверка правой границы
    if (!pVecDataSignal->isEmpty() && estimationBoundaries_.second > minSizeSignals_)
        estimationBoundaries_.second = minSizeSignals_;
}

// Нахождение минимального размера сигнала из группы
int Statistics::calcMinSizeSignals() const {
    if (pVecDataSignal->isEmpty()) return 0; // Проверка на пустоту
    QVector<DataSignal>::const_iterator iter = pVecDataSignal->constBegin();
    int tempMinSize = iter->size(); ++iter;
    for ( ; iter != pVecDataSignal->constEnd(); ++iter)
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
    // По расчетной области
        // Пока текущая левая граница не достигнет конца правой расчетной границы
    for (int s = estimationBoundaries_.first - 1; s < estimationBoundaries_.second && s < minSizeSignals_; ){
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
        if (numeratorA == 0.0 && denominatorA == 0.0)
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
        if (tXSignal == 0.0 && tYSignal == 0.0)
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
    // По расчетной области
        // Пока текущая левая граница не достигнет конца правой расчетной границы
    for (int s = estimationBoundaries_.first - 1; s < estimationBoundaries_.second && s < minSizeSignals_; ){
        int currRightBound = windowProperty.width_;
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

// Расчет всех метрик
void Statistics::calcAllMetrics(){
    for (int iSignal = 0; iSignal != nSize_; ++iSignal)
        calcMetric(iSignal);
}

// Расчет метрики сигнала по индексу
void Statistics::calcMetric(int iSignal){
    static double TOLERANCE = 1e-9; // Точность сравнения
    int nPoints = estimationBoundaries_.second - estimationBoundaries_.first + 1; // Число расчетных точек
    double min = (*pVecDataSignal)[iSignal][estimationBoundaries_.first - 1], max = min; // Min - Max
    double mean = 0, meanSquare = 0; // Mean
    double deviation = 0; // Локальное отклонение
    double elemCur; // Текущий элемент вектора
    // Расчет среднего и экстремумов
    for (int jInd = estimationBoundaries_.first - 1; jInd != estimationBoundaries_.second; ++jInd){
        elemCur = (*pVecDataSignal)[iSignal][jInd]; // Текущий элемент вектора
        mean += elemCur; // Среднее
        if (elemCur > max) max = elemCur; // Максимум
        if (elemCur < min) min = elemCur; // Минимум
    }
    mean /= nPoints; // Нормировка к числу элементов
    // Расчет среднего квадратического
    for (int jInd = estimationBoundaries_.first - 1; jInd != estimationBoundaries_.second; ++jInd)
        meanSquare += qPow( (*pVecDataSignal)[iSignal][jInd] - mean, 2 );
    meanSquare = qSqrt(meanSquare / nPoints); // Извлечение корня и нормировка к числу элементов
    // Расчет локального отклонения
    double elemPrev = (*pVecDataSignal)[iSignal][estimationBoundaries_.first - 1]; // Предыдущий элемент вектора
    double tAmplitude = qAbs(elemPrev - mean); // Инициализация начального отклонения
    for (int jInd = estimationBoundaries_.first; jInd != estimationBoundaries_.second; ++jInd){
        elemCur = (*pVecDataSignal)[iSignal][jInd]; // Текущий элемент вектора
        // Проверка перехода через среднее значение либо на равенство среднему
        if ( (elemCur - mean) * (elemPrev - mean) < 0 || qAbs(elemCur - mean) <= TOLERANCE ){
            deviation += qPow(tAmplitude, 2);
            tAmplitude = 0.0;
        }
        // Нахождение амплитуды
        if ( qAbs(elemCur - mean) > tAmplitude )
            tAmplitude = qAbs(elemCur - mean);
        elemPrev = elemCur; // Сохранение значения для следующей итерации
    }
    deviation = qSqrt(deviation / nPoints); // Извлечение корня и нормировка к числу элементов
    // Занесение значений в контейнеры
    meanSegment_[iSignal] = mean;
    squareMeanSegment_[iSignal] = meanSquare;
    minMaxSegment_[iSignal] = {min, max};
    localDeviationSegment_[iSignal] = deviation;
}

// Методы-обертки для выделения памяти для всех метрик
    // При расширении для всех метрик
void Statistics::allocateAllMetrics(){
    // Проверка необходимости изменения размеров векторов
    if ( nSize_ != meanSegment_.size() ){
        meanSegment_.resize(nSize_);           // Среднее на участке
        squareMeanSegment_.resize(nSize_);     // Среднее квадратическое
        minMaxSegment_.resize(nSize_);         // Минимумы - максимумы на участке
        localDeviationSegment_.resize(nSize_); // Локальное отклонение
    }
}
    // При сжатии для всех метрик
void Statistics::removeAllMetrics(int deleteInd){
    meanSegment_.remove(deleteInd);           // Среднее на участке
    squareMeanSegment_.remove(deleteInd);     // Среднее квадратическое
    minMaxSegment_.remove(deleteInd);         // Минимумы - максимумы на участке
    localDeviationSegment_.remove(deleteInd); // Локальное отклонение
}

// Сохранение метрик по всем сигналам
int Statistics::writeAllMetrics(QString const& dirName, QString const& fileName) const {
    QString fullFilePath = dirName + fileName + ".xlsx"; // Полный путь к файлу
    int exitStatus = 0; // Код возврата
    QXlsx::Document xlsxDocument; // Создание документа расширения .xlsx
    // Установка формата документа
    QXlsx::Format tableFormat;
    tableFormat.setHorizontalAlignment(QXlsx::Format::AlignHCenter); // Выравнивание по горизонтали
    tableFormat.setVerticalAlignment(QXlsx::Format::AlignVCenter); // Выравнивание по вертикали
    xlsxDocument.setColumnWidth(1, 1, 22); // Ширина столбца с именем сигнала
    xlsxDocument.setColumnWidth(2, 6, 15); // Ширина рабочих столбцов
    // Заполнение шапки документа
        // Расчетный диапазон
    QString tempString = "Границы участка: " + numberToString(estimationBoundaries_.first) + " - " + numberToString(estimationBoundaries_.second);
    xlsxDocument.write("A1", tempString);
        // Длина участка
    tempString = "Длина участка: " + numberToString(estimationBoundaries_.second - estimationBoundaries_.first + 1);
    xlsxDocument.write("A2", tempString);
        // Заголовки таблицы
    xlsxDocument.write("A4", "Имя сигнала", tableFormat);
    xlsxDocument.write("B4", "Среднее", tableFormat);
    xlsxDocument.write("C4", "Среднее квадр.", tableFormat);
    xlsxDocument.write("D4", "Минимум", tableFormat);
    xlsxDocument.write("E4", "Максимум", tableFormat);
    xlsxDocument.write("F4", "Локальное откл.", tableFormat);
    int lastLine = 4; // Номер последней линии шапки
    // Запись расчетных данных
    for (int i = 0; i != nSize_; ++i){
        ++lastLine; // Приращение счетчика текущей линии
        xlsxDocument.write("A" + numberToString(lastLine), (*pVecDataSignal)[i].getName(), tableFormat);              // Имя сигнала
        xlsxDocument.write("B" + numberToString(lastLine), numberToString(getMeanSegment(i)), tableFormat);           // Среднее
        xlsxDocument.write("C" + numberToString(lastLine), numberToString(getSquareMeanSegment(i)), tableFormat);     // Среднее квадратическое
        xlsxDocument.write("D" + numberToString(lastLine), numberToString(getMinSegment(i)), tableFormat);            // Минимум
        xlsxDocument.write("E" + numberToString(lastLine), numberToString(getMaxSegment(i)), tableFormat);            // Максимум
        xlsxDocument.write("F" + numberToString(lastLine), numberToString(getLocalDeviationSegment(i)), tableFormat); // Локальное отклонение
    }
    exitStatus = !xlsxDocument.saveAs(fullFilePath); // Сохранение документа
    return exitStatus;
}

// ---- Вспомогательные функции --------------------------------------------------------------------------------

// Получение оконного распределения статистики
    // ArrayRegressionParams
QVector<double> Statistics::getWindowStatisticData(ArrayRegressionParams const& stat, int i, int j) const {
    QVector<double> tData(windowProperty.nWindows_); // Вектор данных для хранения статистик
    for (int s = 0; s != windowProperty.nWindows_; ++s){ // По всем окнам, за исключением среднего
        tData[s] = stat[i][j][s].first; // Угловые коэффициенты
    }
    return tData;
}
    // ArrayStatCharacters
QVector<double> Statistics::getWindowStatisticData(ArrayStatCharacters const& stat, int i, int j) const {
    QVector<double> tData(windowProperty.nWindows_); // Вектор данных для хранения статистик
    for (int s = 0; s != windowProperty.nWindows_; ++s){ // По всем окнам, за исключением среднего
        tData[s] = stat[i][j][s];
    }
    return tData;
}

// Перевод числа в строку
QString numberToString(int const& value){ return QString::number(value); }
QString numberToString(double const& value, QString const& delimiter) { return QString::number(value).replace(".", delimiter); }

// -------------------------------------------------------------------------------------------------------------
