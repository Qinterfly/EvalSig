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
    // Инициализация векторов, определяющих уровни
    lowBoundLevels_ = {0}; upperBoundLevels_ = {0};
    indLevels_ = {0};
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
    bool isChanged = false; // Флаг изменения границ
    if (rBound == -1) rBound = ptrAccel_->size(); // Правая граница по умолчанию
    --lBound; --rBound; // Сдвиг границ к индексам
    if (lBound != calculationInd_.first || rBound != calculationInd_.second) isChanged = true; // Если хотя бы одна граница изменилась
    calculationInd_ = {lBound, rBound};
    // Если уровни до этого уже были созданы и границы сменились
    if ( nLevels != 0 && isChanged ){
        createLevels();
    }
}

// Файловые методы
     // Сохранение перемещений
int DivisionDataSignal::writeDisplacement(QString const& path, QString const& fileName){
    return displacement_.writeDataFile(path, fileName, calculationInd_.first, calculationInd_.second);
}
