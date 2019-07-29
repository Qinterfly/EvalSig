#include <QDebug>
#include <thread>
#include "DivisionDataSignal.h"
#include "core/NumericalFunctions.h"
#include "QtMath"

static const int MAX_THREAD_NUM = 8; // Максимальное число потоков

// Конструктор
DivisionDataSignal::DivisionDataSignal(DataSignal const& dataSignal, double levelStep, double overlapFactor, double smoothIntegFactor,
                       double smoothApproxFactor, double truncatePercent, int lEstimationBound, int rEstimationBound)
    : levelStep_(levelStep), smoothIntegFactor_(smoothIntegFactor), smoothApproxFactor_(smoothApproxFactor),
    truncatePercent_(truncatePercent), accel_(dataSignal)
{
    overlapFactor_ = overlapFactor != 0.0 ? overlapFactor : 1; // Обработка коэффициента перекрытия
    setCalculationInd(lEstimationBound, rEstimationBound); // Задание расчетных границ
    accel_.normalize(FIRST); // Приводим ускорения к нулю
    // Инициализация векторов, определяющих уровни
    lowBoundLevels_ = {0}; upperBoundLevels_ = {0};
    indLevels_ = {0};
    // Нахождение перемещений по сигналу ускорения
    displacement_ = integrate(dataSignal, 2, smoothIntegFactor_)[1];
    approxDisplacement_ = approximateSmoothSpline(displacement_, smoothApproxFactor_); // Аппроксимация перемещений
    createLevels(); // Создание расчетных уровней
    callMultiThread(&DivisionDataSignal::assignLevels); // Назначить уровни в многопоточном режиме
    callMultiThread(&DivisionDataSignal::truncateLevels); // Усечь уровни
    derivativeLevels(); // Вычисление производных
}

// Создание расчетных уровней
void DivisionDataSignal::createLevels(){
    QVector<double> const& data = approxDisplacement_.getData(); // Данные перемещений
    auto [min, max] = minMaxVec(data, calculationInd_.first, calculationInd_.second);
    double mean = meanVec(data, calculationInd_.first, calculationInd_.second);
    nLevels_ = 1; // Один уровень гарантирован
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
        ++nLevels_;
    }
    // Запись границ уровней
    lowBoundLevels_.resize(nLevels_); // Выделяем память для нижних границ
    upperBoundLevels_.resize(nLevels_); // Выделяем память для верхних границ
    indLevels_.resize(nLevels_); // Выделяем память для индексов уровней
    for (int i = 0; i != nLevels_; ++i){
        lowBoundLevels_[i] = min + i * overlapFactor_ * levelStep_; // Нижние границы
        upperBoundLevels_[i] = lowBoundLevels_[i] + levelStep_; // Нижние границы
    }
    // Нумерация уровней
    int indZeroLevel = qFloor(nLevels_ / 2.0); // Индекс нулевого уровня
    indLevels_[indZeroLevel] = 0;
        // Нижние уровни
    for (int i = indZeroLevel - 1; i >= 0; --i)
        indLevels_[i] = i - indZeroLevel;
        // Верхние уровни
    for (int i = indZeroLevel + 1; i != nLevels_; ++i)
        indLevels_[i] = i - indZeroLevel;
    // Выделение памяти для частей
    parTime_.resize(nLevels_);
    parAccel_.resize(nLevels_);
    parFlags_.resize(nLevels_);
    parInd_.resize(nLevels_);
    lengthLevels_.resize(nLevels_);
    nFragmentLevels_.resize(nLevels_);
}

// Назначить уровни
void DivisionDataSignal::assignLevels(int firstLevelInd, int lastLevelInd){
    QVector<double> const& data = approxDisplacement_.getData(); // Данные перемещений
    if (lastLevelInd == -1) lastLevelInd = nLevels_ - 1; // Обработка обратной индексации
    for (int i = firstLevelInd; i <= lastLevelInd; ++i) { // По группе уровней
        // Оценка размеров
        int lenLevel = 0; // Длина сигнала на заданном уровне
        bool isFragment = 0; // Флаг окончания фрагмента
        int iFragment = 0; // Счетчик фрагментов
        for (int j = calculationInd_.first; j <= calculationInd_.second; ++j){
            if ( data[j] >= lowBoundLevels_[i] && data[j] <= upperBoundLevels_[i] ){
                isFragment = 1;
                ++lenLevel;
            } else if (isFragment){
                isFragment = 0;
                ++iFragment;
            }
        }
        if (isFragment) ++iFragment; // Если последний фрагмент закончился на последнем значении сигнала
        lengthLevels_[i] = lenLevel;
        nFragmentLevels_[i] = iFragment;
        // Выделение памяти для всех фрагментов на уровне
        parTime_[i].resize(lenLevel); // Время
        parAccel_[i].resize(lenLevel); // Ускорения
        parFlags_[i].resize(lenLevel); // Флаги концов
        parInd_[i].resize(iFragment); // Индексы концов
        // Заполнение векторов
        lenLevel = 0; iFragment = 0; isFragment = 0;
        for (int j = calculationInd_.first; j <= calculationInd_.second; ++j){
            if ( data[j] >= lowBoundLevels_[i] && data[j] <= upperBoundLevels_[i] ){
                parTime_[i][lenLevel] = j + 1; // Время
                parAccel_[i][lenLevel] = accel_[j]; // Ускорение
                isFragment = 1;
                ++lenLevel;
            } else if (isFragment){
                parFlags_[i][lenLevel - 1] = 1; // Флаг конца
                parInd_[i][iFragment] = lenLevel - 1; // Индекс конца
                isFragment = 0;
                ++iFragment;
            }
        }
        // Если последний фрагмент закончился на последнем значении сигнала
        if (isFragment){
            parFlags_[i][lenLevel - 1] = 1;
            parInd_[i][iFragment] = lenLevel - 1;
        }
    }
}

// Усечение коротких фрагментов
void DivisionDataSignal::truncateLevels(int firstLevelInd, int lastLevelInd){
    if (lastLevelInd == -1) lastLevelInd = nLevels_ - 1; // Обработка обратной индексации
    for (int i = firstLevelInd; i <= lastLevelInd; ++i){
        int nFragment = nFragmentLevels_[i]; // Число фрагментов на уровне
        int lastIndex = 0; // Индекс конца фрагмента
        int lenFragment = 0; // Длина фрагмента
        int maxLenFragment = 0; // Максимальная длина фрагмента
        // Нахождение максимальной длины фрагмента
        for (int j = 0; j != nFragment; ++j){
            lenFragment = parInd_[i][j] - lastIndex + 1;
            if (lenFragment > maxLenFragment) maxLenFragment = lenFragment; // Запись максимальной длины
            lastIndex = parInd_[i][j] + 1; // Запоминаем индекс конца фрагмента
        }
        int limTruncate = qCeil(maxLenFragment * truncatePercent_); // Длина среза
        // Срез фрагментов
        lastIndex = 0;
        int endFixDataIndex = 0; // Индекс конца отфильтрованных данных
        int endFixFragmentIndex = 0; // Индекс конца усеченного фрагмента
        int endFragmentInd = 0; // Индекс конца фрагмента
        double meanFragment = 0; // Среднее значение фрагмента
        for (int j = 0; j != nFragment; ++j){
            endFragmentInd = parInd_[i][j];
            lenFragment = endFragmentInd - lastIndex + 1;
            if (lenFragment > limTruncate){ // Если длина фрагмента достаточна
                meanFragment = meanVec(parAccel_[i], lastIndex, endFragmentInd); // Среднее на фрагменте
                // Пределы
                auto [minFragment, maxFragment] = minMaxVec(parAccel_[i], lastIndex, endFragmentInd); // Получение минимума и максимума
                minFragment = qAbs(minFragment - meanFragment); // Модуль минимума со сдвигом
                maxFragment = qAbs(maxFragment - meanFragment); // Модуль максимума со сдвигом
                if ( minFragment > maxFragment ) maxFragment = minFragment; // Сравнение по модулю
                if ( maxFragment == 0.0 ) maxFragment = 1.0; // Для нулевых фрагментов
                // Перезапись данных
                for (int k = lastIndex; k <= endFragmentInd; ++k){
                    parTime_[i][endFixDataIndex] = parTime_[i][k];
                    parAccel_[i][endFixDataIndex] = (parAccel_[i][k] - meanFragment) / maxFragment;
                    parFlags_[i][endFixDataIndex] = parFlags_[i][k];
                    ++endFixDataIndex;
                }
                parInd_[i][endFixFragmentIndex] = endFixDataIndex - 1; // Записываем индекс конца усеченного фрагмента
                ++endFixFragmentIndex;
            }
            lastIndex = endFragmentInd + 1; // Запоминаем индекс конца фрагмента
        }
        lengthLevels_[i] = endFixDataIndex; // Длина уровня после усечения
        nFragmentLevels_[i] = endFixFragmentIndex; // Число фрагментов после усечения
    }
}

// Вычисление производных
void DivisionDataSignal::derivativeLevels(int firstLevelInd, int lastLevelInd){
    /* Правая конечная разность */
}

// Вызов метода в многопоточном режиме
void DivisionDataSignal::callMultiThread(void (DivisionDataSignal::*method)(int, int)){
    int nLevelPerThread = nLevels_ / MAX_THREAD_NUM ; // Число уровней на поток
    int residue = nLevels_ % MAX_THREAD_NUM; // Остаток
    int nThread = MAX_THREAD_NUM; // Реальное число потоков
    // Если потоков больше чем задач
    if (nLevelPerThread == 0)
        nThread = nLevels_;
    std::thread thread[nThread]; // Создание потоков
    int firstLevel = 0; // Первый уровень
    int lastLevel = 0; // Последний уровень
    for (int i = 0; i != nThread; ++i){
        // Если остаток не распределен
        if (residue > 0){
            lastLevel = firstLevel + nLevelPerThread + 1; // + одна задача к норме
            --residue;
        } else
            lastLevel = firstLevel + nLevelPerThread;
        thread[i] = std::thread(method, this, firstLevel, lastLevel - 1);
        firstLevel = lastLevel;
    }
    // Блокируем основной поток до выполнения вызванных
    for (int i = 0; i != nThread; ++i)
        thread[i].join();
}

// Задание индексов расчетных границ
void DivisionDataSignal::setCalculationInd(int lBound, int rBound){
    bool isChanged = false; // Флаг изменения границ
    if (rBound == -1) rBound = accel_.size(); // Правая граница по умолчанию
    --lBound; --rBound; // Сдвиг границ к индексам
    if (lBound != calculationInd_.first || rBound != calculationInd_.second) isChanged = true; // Если хотя бы одна граница изменилась
    calculationInd_ = {lBound, rBound};
    // Если уровни до этого уже были созданы и границы сменились
    if ( nLevels_ != 0 && isChanged ){
        createLevels(); // Сформировать новые уровни
        callMultiThread(&DivisionDataSignal::assignLevels); // Назначить уровни в многопоточном режиме
        callMultiThread(&DivisionDataSignal::truncateLevels); // Усечь уровни
    }
}

// Файловые методы
     // Сохранение перемещений
int DivisionDataSignal::writeDisplacement(QString const& path, QString const& fileName){
    return displacement_.writeDataFile(path, fileName, calculationInd_.first, calculationInd_.second);
}

// Сохранение аппроксимированных перемещений
int DivisionDataSignal::writeApproxDisplacement(QString const& path, QString const& fileName){
    return approxDisplacement_.writeDataFile(path, fileName, calculationInd_.first, calculationInd_.second);
}


