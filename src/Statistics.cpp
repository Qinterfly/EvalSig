#include "Statistics.h"

// Нахождение минимального размера сигнала из группы
int Statistics::getMinSize(QVector<DataSignal> const& vecData){
    auto iter = vecData.begin();
    int tempMinSize = iter->size(); ++iter;
    for ( ; iter != vecData.end(); ++iter)
        if (iter->size() < tempMinSize) tempMinSize = iter->size();
    return tempMinSize;
}

// Выделение памяти для полей структуры
template<typename T>
void Statistics::initAllocateField(T& field){
    field.resize(nSize_); // Строки
    for (int i = 0; i != nSize_; ++i){ // Столбцы
        field[i].resize(nSize_);
        for (int j = 0; j != nSize_; ++j)
            field[i][j].resize(nWindows_);
    }
}

// Расчет коэффиицентов
void Statistics::computeParams(QVector<DataSignal> const& vecData){
    // ~ гарантируется, что widthTimeWindow_ <= minSizeSignals_
    // Расчет шага по времени
    int shiftWindow = qCeil( widthTimeWindow_ * (1 - overlapFactor_) );
    for (int i = 0; i != nSize_; ++i){ // По всем сигналам
        for (int j = 0; j != nSize_; ++j){
            int currWindow = 0; // Номер текущего окна
            // По всем окнам
            for (int s = 0; s < minSizeSignals_; ){ // Пока левая граница не достигнет конца сигнала
                int currRightBound = widthTimeWindow_;
                if (currRightBound + s > minSizeSignals_) // Проверка правой границы
                    currRightBound = minSizeSignals_ - s;
                // Нахождение средних значений
                double meanX = 0, meanY = 0;
                for (int k = 0; k != currRightBound; ++k){
                    meanX += vecData[i][s + k];
                    meanY += vecData[j][s + k];
                }
                meanX /= widthTimeWindow_;
                meanY /= widthTimeWindow_;
                // Нахождение параметров линейной регрессии
                double numeratorA = 0, denominatorA = 0; // Числитель и знаменатель углового коэффициента
                for (int k = 0; k != currRightBound; ++k){
                    numeratorA += (vecData[i][s + k] - meanX) * (vecData[j][s + k] - meanY);
                    denominatorA += qPow(vecData[i][s + k] - meanX, 2);
                }
                regressionParams_[i][j][currWindow].first = numeratorA / denominatorA; // Угловой коэффициент
                regressionParams_[i][j][currWindow].second = meanY - regressionParams_[i][j][currWindow].first * meanX; // Смещение прямой
                double alpha = qAtan(regressionParams_[i][j][currWindow].first); // Угол наклона прямой
                double tSumDistance = 0; // Подсумма дистанции рассеяния
                double tXSignal = 0, tYSignal = 0; // Для амплитуды рассеяния
                for (int k = 0; k != currRightBound; ++k){
                    // Вычисление регрессионной функции
                    double tLinearRegressionFun = regressionParams_[i][j][currWindow].first * vecData[i][s + k]+ regressionParams_[i][j][currWindow].second;
                    // Вычисление подсуммы дистанции рассеяния
                    tSumDistance += 1. / minSizeSignals_* qFabs(vecData[j][s + k] - tLinearRegressionFun) * qCos(alpha);
                    // Для амплитуды рассеяния
                    tXSignal += qAbs(vecData[i][s + k] - meanX);
                    tYSignal += qAbs(vecData[j][s + k] - meanY);
                }
                distanceScatter_[i][j][currWindow] = tSumDistance; // Дистанция рассеяния
                amplitudeScatter_[i][j][currWindow] = tSumDistance * minSizeSignals_ / qSqrt(qPow(tXSignal, 2) + qPow(tYSignal, 2)); // Амплитуда рассеяния
                s += shiftWindow; // Сдвиг левой границы окна
                currWindow += 1; // Приращение счетчика окон
            }
        }
    }

    // Вычисление коэффициентов подобия
    for (int i = 0; i != nSize_; ++i){ // По всем сигналам
        for (int j = 0; j != nSize_; ++j){
            int currWindow = 0; // Номер текущего окна
            // По всем окнам
            for (int s = 0; s < minSizeSignals_; ){ // Пока левая граница не достигнет конца сигнала
                int currRightBound = widthTimeWindow_;
                if (currRightBound + s > minSizeSignals_) // Проверка правой границы
                    currRightBound = minSizeSignals_ - s;
                similarityCoeffs_[i][j][currWindow] = qSqrt(regressionParams_[i][j][currWindow].first * regressionParams_[j][i][currWindow].first);
                s += shiftWindow; // Сдвиг левой границы окна
                currWindow += 1; // Приращение счетчика окон
            }
        }
    }
}

// Конструктор
Statistics::Statistics(QVector<DataSignal> const& vecData, int widthTimeWindow, double overlapFactor)
    : nSize_(vecData.size()), minSizeSignals_(getMinSize(vecData)),
      widthTimeWindow_(widthTimeWindow), overlapFactor_(overlapFactor)
{
    nWindows_ = qCeil(minSizeSignals_ / ( widthTimeWindow_ * (1 - overlapFactor_) ) );
    // Выделение памяти для хранения полей
    initAllocateField(regressionParams_); // Параметры линейной регрессии
    initAllocateField(distanceScatter_); // Дистанция рассеяния
    initAllocateField(similarityCoeffs_); // Коэффициенты подобия сигналов
    initAllocateField(amplitudeScatter_); // Амплитуда рассеяния
    computeParams(vecData);
}

