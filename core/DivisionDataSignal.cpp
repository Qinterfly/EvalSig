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
    nLevels = 1; // Один уровень гарантирован
    // Обработка единственного уровня
    if (max <= mean + levelStep_ / 2 && min >= mean - levelStep_ / 2){
        // Задание размеров вектора
        lowBoundLevels_.resize(1); // Нижняя граница
        upperBoundLevels_.resize(1); // Верхняя граница
        indLevels_.resize(1); // Индекс
        // Заполнение
        lowBoundLevels_[0] = mean - levelStep_ / 2;
        upperBoundLevels_[0] = mean + levelStep_ / 2;
        indLevels_[0] = 0;
        return;
    }
    // Разбивка по уровням от минимума сигнала
    // Оценка числа уровней
    double lastUpperBound = min + levelStep_;
    while (lastUpperBound <= max){
        lastUpperBound += overlapFactor_ * levelStep_; // Сдвиг верхней границы
        ++nLevels;
    }
    // Запись границ уровней
    lowBoundLevels_.resize(nLevels); // Выделяем память для нижних границ
    upperBoundLevels_.resize(nLevels); // Выделяем память для верхних границ
    indLevels_.resize(nLevels); // Выделяем память для индексов уровней
    for (int i = 0; i != nLevels; ++i){
        lowBoundLevels_[i] = min + i * overlapFactor_ * levelStep_; // Нижние границы
        upperBoundLevels_[i] = min + (i + 1) * overlapFactor_ * levelStep_; // Нижние границы
    }
    // Нумерация уровней
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
    // Если уровни до этого уже были созданы
    if (nLevels != 0){
        createLevels();
    }
}

// Файловые методы
     // Сохранение перемещений
int DivisionDataSignal::writeDisplacement(QString const& path, QString const& fileName){
    return displacement_.writeDataFile(path, fileName, calculationInd_.first, calculationInd_.second);
}
