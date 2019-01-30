#include "Statistics.h"

// Конструктор TimeWindowProperty
TimeWindowProperty::TimeWindowProperty(int width, double overlapFactor, int sizeSignals)
    : width_(width), overlapFactor_(overlapFactor) {
    nWindows_ = qCeil(sizeSignals / ( width_ * (1 - overlapFactor_) ) ); // Число окон
    shiftWindow_ = qCeil( width_ * (1 - overlapFactor_) ); // Смещение окна по времени
}

// Конструктор Statistics
Statistics::Statistics(QVector<DataSignal> const& vecDataSignal, int widthTimeWindow, double overlapFactor)
    : nSize_(vecDataSignal.size()), minSizeSignals_(calcMinSizeSignals(vecDataSignal)),
      windowProperty(widthTimeWindow, overlapFactor, minSizeSignals_)
{
    allocateAllFields(0, nSize_); // Выделение памяти для хранения полей
    fullCompute(vecDataSignal); // Полный расчет матрицы характеристик
}

// Интерфейс пользователя
int Statistics::size() const { return nSize_; } // Текущий размер матрицы статистик
bool Statistics::isEmpty() const { return size() == 0; } // Проверка на пустоту
int Statistics::minSizeSignals() const {return minSizeSignals_; }; // Минимальная длина сигнала из группы
    // Добавление сигнала
bool Statistics::addSignal(QVector<DataSignal> & vecDataSignal, DataSignal const& dataSignal){
    int sizeSignal = dataSignal.size(); // Длина сигнала
    if (sizeSignal < windowProperty.width_){ // Обработка исключения
        qDebug() << "Ширина окна превышает длину сигнала" << dataSignal.getName();
        return 1;
    }
    vecDataSignal.push_back(dataSignal);   // Добавление объекта в вектор сигналов
    allocateAllFields(nSize_, nSize_ + 1); // Инциализация дополнительных полей
    ++nSize_; // Увеличение размера матрицы статистик
    // Оценка необходимости полного пересчета матрицы
    if (sizeSignal >= minSizeSignals_ && minSizeSignals_ != 0)
        partialCompute(vecDataSignal); // Вызов метода частичного пересчета
    else { // Полный пересчет
        minSizeSignals_ = sizeSignal; // Запись новой наименьшой длины сигнала
        fullCompute(vecDataSignal); // Вызов метода полного пересчета
    }
    return 0;
}
    // Удаление сигнала
bool Statistics::removeSignal(QVector<DataSignal> & vecDataSignal, int deleteInd){
    if (isEmpty()) { qDebug() << "Объект статистик пуст"; return 1; } // Проверка на пустоту
    if (deleteInd > nSize_ - 1){ qDebug() << "Попытка удаления несуществующего элемента"; return 1; } // Проверка на возможность удаления
    vecDataSignal.remove(deleteInd); // Удаление объекта из вектора сигналов
    removeAllFields(deleteInd); // Удаление статистик, связанных с объектов
    --nSize_; // Уменьшение размера матрицы
    int tempMinSizeSignals = calcMinSizeSignals(vecDataSignal); // Получение нового минимального размера группы сигналов
    if (tempMinSizeSignals != minSizeSignals_){ // Если после удаление минимальный размер сигналов изменился
        minSizeSignals_ = tempMinSizeSignals; // Запись нового размера
        fullCompute(vecDataSignal); // Вызов метода полного пересчета
    }
    return 0;
}

// Выделение памяти для полей структуры
    // При расширении объекта
template<typename T>
void Statistics::allocateField(T& field, int beginColInd, int fullSize){
    field.resize(fullSize); // Строки
    for (int i = 0; i != fullSize; ++i){ // Столбцы
        field[i].resize(fullSize);
        for (int j = beginColInd; j != fullSize; ++j)
            field[i][j].resize(windowProperty.nWindows_);
    }
    // В случае расширения матрицы одним сигналом дополнительно
    // инициализируем последнюю строку, за исключением диагонального элемента
    if (beginColInd == fullSize - 1)
        for (int j = 0; j != fullSize - 1; ++j)
            field[fullSize - 1][j].resize(windowProperty.nWindows_);
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
}

// При сжатии для всех полей
void Statistics::removeAllFields(int deleteInd){
    removeField(regressionParams_, deleteInd);   // Параметры линейной регрессии
    removeField(distanceScatter_, deleteInd);    // Дистанция рассеяния
    removeField(similarityCoeffs_, deleteInd);   // Коэффициенты подобия сигналов
    removeField(amplitudeScatter_, deleteInd);   // Амплитуда рассеяния
}

// Нахождение минимального размера сигнала из группы
int Statistics::calcMinSizeSignals(QVector<DataSignal> const& vecDataSignal){
    if (vecDataSignal.isEmpty()) return 0; // Проверка на пустоту
    auto iter = vecDataSignal.begin();
    int tempMinSize = iter->size(); ++iter;
    for ( ; iter != vecDataSignal.end(); ++iter)
        if (iter->size() < tempMinSize) tempMinSize = iter->size();
    return tempMinSize;
}

// Полный расчет статистик
void Statistics::fullCompute(QVector<DataSignal> const& vecDataSignal){
    // ~ гарантируется, что widthTimeWindow_ <= minSizeSignals_ 
    // Расчет регрессионных параметров, дистанций и амплитуд рассеяния
    for (int i = 0; i != nSize_; ++i)
        for (int j = 0; j != nSize_; ++j)
            calcDistanceAmplitudeRegression(vecDataSignal, windowProperty.shiftWindow_, i, j);
    // Расчет коэффициентов подобия
    for (int i = 0; i != nSize_; ++i) // По всем сигналам
        for (int j = 0; j != nSize_; ++j)
            calcSimilarity(windowProperty.shiftWindow_, i, j);
}

// Частичный пересчет статистик (при добавлении одного сигнала)
void Statistics::partialCompute(QVector<DataSignal> const& vecDataSignal){
    // ~ гарантируется, что propertyWindow.width_ <= minSizeSignals_
    int shiftWindow = qCeil( windowProperty.width_ * (1 - windowProperty.overlapFactor_) ); // Смещение окна по времени
    // Расчет регрессионных параметров, дистанций и амплитуд рассеяния
        // По последнему столбцу
    for (int i = 0; i != nSize_; ++i)
        calcDistanceAmplitudeRegression(vecDataSignal, shiftWindow, i, nSize_ - 1);
        // По последней строке, за исключением диагонального элемента
    for (int j = 0; j != nSize_ - 1; ++j)
        calcDistanceAmplitudeRegression(vecDataSignal, shiftWindow, nSize_ - 1, j);
    // Расчет коэффициентов подобия [циклы объединить нельзя]
        // По последнему столбцу
    for (int i = 0; i != nSize_; ++i)
        calcSimilarity(shiftWindow, i, nSize_ - 1);
    // По последней строке, за исключением диагонального элемента
    for (int j = 0; j != nSize_ - 1; ++j)
        calcSimilarity(shiftWindow, nSize_ - 1, j);
}

// Тело цикла пересчета для дистанций, амплитуд и регрессионных параметров
void Statistics::calcDistanceAmplitudeRegression(QVector<DataSignal> const& vecDataSignal, int shiftWindow, int i, int j){
    int currWindow = 0; // Номер текущего окна
    // По всем окнам
    for (int s = 0; s < minSizeSignals_; ){ // Пока левая граница не достигнет конца сигнала
        int currRightBound = windowProperty.width_;
        if (currRightBound + s > minSizeSignals_) // Проверка правой границы
            currRightBound = minSizeSignals_ - s;
        // Нахождение средних значений
        double meanX = 0, meanY = 0;
        for (int k = 0; k != currRightBound; ++k){
            meanX += vecDataSignal[i][s + k];
            meanY += vecDataSignal[j][s + k];
        }
        meanX /= windowProperty.width_;
        meanY /= windowProperty.width_;
        // Нахождение параметров линейной регрессии
        double numeratorA = 0, denominatorA = 0; // Числитель и знаменатель углового коэффициента
        for (int k = 0; k != currRightBound; ++k){
            numeratorA += (vecDataSignal[i][s + k] - meanX) * (vecDataSignal[j][s + k] - meanY);
            denominatorA += qPow(vecDataSignal[i][s + k] - meanX, 2);
        }
        regressionParams_[i][j][currWindow].first = numeratorA / denominatorA; // Угловой коэффициент
        regressionParams_[i][j][currWindow].second = meanY - regressionParams_[i][j][currWindow].first * meanX; // Смещение прямой
        double alpha = qAtan(regressionParams_[i][j][currWindow].first); // Угол наклона прямой
        double tSumDistance = 0; // Подсумма дистанции рассеяния
        double tXSignal = 0, tYSignal = 0; // Для амплитуды рассеяния
        for (int k = 0; k != currRightBound; ++k){
            // Вычисление регрессионной функции
            double tLinearRegressionFun = regressionParams_[i][j][currWindow].first * vecDataSignal[i][s + k]+ regressionParams_[i][j][currWindow].second;
            // Вычисление подсуммы дистанции рассеяния
            tSumDistance += 1. / minSizeSignals_* qFabs(vecDataSignal[j][s + k] - tLinearRegressionFun) * qCos(alpha);
            // Для амплитуды рассеяния
            tXSignal += qAbs(vecDataSignal[i][s + k] - meanX);
            tYSignal += qAbs(vecDataSignal[j][s + k] - meanY);
        }
        distanceScatter_[i][j][currWindow] = tSumDistance; // Дистанция рассеяния
        amplitudeScatter_[i][j][currWindow] = tSumDistance * minSizeSignals_ / qSqrt(qPow(tXSignal, 2) + qPow(tYSignal, 2)); // Амплитуда рассеяния
        s += shiftWindow; // Сдвиг левой границы окна
        currWindow += 1; // Приращение счетчика окон
    }
}

// Тело цикла для расчета коэффициентов подобия
void Statistics::calcSimilarity(int shiftWindow, int i, int j){
    int currWindow = 0; // Номер текущего окна
    // По всем окнам
    for (int s = 0; s < minSizeSignals_; ){ // Пока левая граница не достигнет конца сигнала
        int currRightBound = windowProperty.width_;;
        if (currRightBound + s > minSizeSignals_) // Проверка правой границы
            currRightBound = minSizeSignals_ - s;
        similarityCoeffs_[i][j][currWindow] = qSqrt(regressionParams_[i][j][currWindow].first * regressionParams_[j][i][currWindow].first);
        s += shiftWindow; // Сдвиг левой границы окна
        currWindow += 1; // Приращение счетчика окон
    }
}

