#include "DivisionDataSignal.h"
#include "core/NumericalFunctions.h"
#include "QtMath"

// Конструктор
DivisionDataSignal::DivisionDataSignal(DataSignal const& dataSignal, double levelStep, double overlapFactor, double smoothFactor,
                                       int leftEstimationBound, int rightEstimationBound)
    : levelStep_(levelStep), smoothFactor_(smoothFactor), ptrAccel_(&dataSignal)
{
    overlapFactor_ = overlapFactor != 0.0 ? overlapFactor : 1; // Обработка коэффициента перекрытия
    setCalculationInd(leftEstimationBound, rightEstimationBound); // Задание расчетных границ
    // Нахождение перемещений по сигналу ускорения
    displacement_ = integrate(dataSignal, 2, smoothFactor_)[1];
    createLevels(); // Создание расчетных уровней
}

// Создание расчетных уровней
void DivisionDataSignal::createLevels(){
    QVector<double> const& data = displacement_.getData(); // Данные перемещений
    auto [min, max] = minMaxVec(data, calculationInd_.first, calculationInd_.second);
    double mean = meanVec(data, calculationInd_.first, calculationInd_.second);
    // Обработка единственного уровня
    if (max <= mean + levelStep_ / 2 && min >= mean - levelStep_ / 2){
        lowBoundLevels_.push_back(mean - levelStep_ / 2);
        upperBoundLevels_.push_back(mean + levelStep_ / 2);
        indLevels_.push_back(0);
        return;
    }
    // Разбивка по уровням от минимума сигнала
        // Определяем первый уровень
    lowBoundLevels_.push_back(min);
    upperBoundLevels_.push_back(min + levelStep_);
    while(true){
        if (upperBoundLevels_.last() >= max) break; // До тех пор, пока верхняя граница не достигла максимума
        lowBoundLevels_.push_back(lowBoundLevels_.last() + overlapFactor_ * levelStep_); // Нижние границы
        upperBoundLevels_.push_back(upperBoundLevels_.last() + overlapFactor_ * levelStep_); // Верхние границы
    }
    // Нумерация уровней
    nLevels = lowBoundLevels_.size(); // Число уровней
    indLevels_.resize(nLevels); // Выделяем память для индексов уровней
    int indZeroLevel = qFloor(nLevels / 2.0); // Индекс нулевого уровня
    indLevels_[indZeroLevel] = 0;
        // Нижние уровни
    for (int i = indZeroLevel - 1; i >= 0; --i)
        indLevels_[i] = i - indZeroLevel;
        // Верхние уровни
    for (int i = indZeroLevel + 1; i != nLevels; ++i)
        indLevels_[i] = i - indZeroLevel;
}


// Задание индексов расчетных границ
void DivisionDataSignal::setCalculationInd(int lBound, int rBound){
    if (rBound == -1) rBound = ptrAccel_->size(); // Правая граница по умолчанию
    if (lBound > rBound) lBound = 1; // Проверка корректности границ
    calculationInd_ = {lBound - 1, rBound - 1};
}

// Файловые методы
     // Сохранение перемещений
int DivisionDataSignal::writeDisplacement(QString const& path, QString const& fileName){
    return displacement_.writeDataFile(path, fileName, calculationInd_.first, calculationInd_.second);
}
