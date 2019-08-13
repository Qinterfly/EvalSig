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
    truncatePercent_(truncatePercent), accel_(dataSignal),
    partsAccel(accel_), partsDisplacement(displacement_), partsAccelGlued(accel_), // Части ускорений и перемещений
    // Монотонные части
    partsAccelIncrease(partsAccel, partsDisplacement), partsAccelNeutral(partsAccel, partsDisplacement),
    partsAccelDecrease(partsAccel, partsDisplacement)
{
    overlapFactor_ = overlapFactor != 0.0 ? overlapFactor : 1; // Обработка коэффициента перекрытия
    setCalculationInd(lEstimationBound, rEstimationBound); // Задание расчетных границ
    accel_.normalize(FIRST); // Приводим ускорения к нулю
    // Инициализация векторов, определяющих уровни
    lowBoundLevels_ = {0}; upperBoundLevels_ = {0};
    indLevels_ = {0};
    // Нахождение перемещений по сигналу ускорения
    displacement_ = integrate(dataSignal, 2, smoothIntegFactor_)[1];
    displacement_.normalize(FIRST); // Приводим перемещения к нулю
    approxDisplacement_ = approximateSmoothSpline(displacement_, smoothApproxFactor_); // Аппроксимация перемещений
    vecPartsAccelMonotone = {&partsAccelIncrease, &partsAccelNeutral, &partsAccelDecrease}; // Запись адресов монотонных частей
    calculate(); // Расчет уровней
}

// Управляющий расчетный метод
void DivisionDataSignal::calculate(){
    createLevels(); // Создание расчетных уровней
    // Выделение памяти для частей по уровням
    partsAccel.resizeAll(nLevels_); // Ускорения
    partsDisplacement.resizeAll(nLevels_); // Перемещения
    partsAccelIncrease.resizeAll(nLevels_); // Возрастающие части ускорений
    partsAccelNeutral.resizeAll(nLevels_); // Нейтральные части ускорений
    partsAccelDecrease.resizeAll(nLevels_); // Убывающие части ускорений
    // Получение ссылок на базовые классы
    PartsObject & partsBaseDisplacement = partsDisplacement;    // Перемещения
    PartsObject & partsBaseAccel = partsAccel;                  // Ускорения
    PartsObject & partsBaseAccelIncrease = partsAccelIncrease;  // Возрастающие ускорения
    PartsObject & partsBaseAccelDecrease = partsAccelDecrease;  // Убывающие ускорения
    PartsObject & partsBaseAccelNeutral = partsAccelNeutral;    // Нейтральные ускорения
    // Выделение частей
    callMultiThread(partsDisplacement, &DivisionDataSignal::assignLevels); // Перемещений
    partsAccel.constructByImage(partsDisplacement);                        // Ускорений по образу
    // Усечение уровней
    callMultiThread(partsBaseDisplacement, &DivisionDataSignal::truncateLevels); // Перемещений
    callMultiThread(partsBaseAccel, &DivisionDataSignal::truncateLevels);        // Ускорений
    // Расчет монотонных частей
    callMultiThread(vecPartsAccelMonotone, &DivisionDataSignal::constructMonotoneLevels);
    // Вычисление производных
    callMultiThread(partsBaseAccel, &DivisionDataSignal::derivativeLevels);         // Ускорений
    callMultiThread(partsBaseAccelIncrease, &DivisionDataSignal::derivativeLevels); // Возрастающих ускорений
    callMultiThread(partsBaseAccelDecrease, &DivisionDataSignal::derivativeLevels); // Убывающих ускорений
    callMultiThread(partsBaseAccelNeutral, &DivisionDataSignal::derivativeLevels);  // Нейтральных ускорений
    // Вычисление склеек (TODO)
    //    glueLevels(partsBaseAccel);
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
    lowBoundLevels_.resize(nLevels_);   // Выделяем память для нижних границ
    upperBoundLevels_.resize(nLevels_); // Выделяем память для верхних границ
    indLevels_.resize(nLevels_);        // Выделяем память для индексов уровней
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
}

// Назначить уровни
void DivisionDataSignal::assignLevels(PartsSignal & partsSignal, int firstLevelInd, int lastLevelInd){
    if (lastLevelInd == -1) lastLevelInd = nLevels_ - 1; // Обработка обратной индексации
    for (int i = firstLevelInd; i <= lastLevelInd; ++i) { // По группе уровней
        // Оценка размеров
        int lenLevel = 0; // Длина сигнала на заданном уровне
        bool isFragment = false; // Флаг окончания фрагмента
        int iFragment = 0; // Счетчик фрагментов
        for (int j = calculationInd_.first; j <= calculationInd_.second; ++j){
            if ( partsSignal.signal_[j] >= lowBoundLevels_[i] && partsSignal.signal_[j] <= upperBoundLevels_[i] ){
                isFragment = true;
                ++lenLevel;
            } else if (isFragment){
                isFragment = false;
                ++iFragment;
            }
        }
        if (isFragment) ++iFragment; // Если последний фрагмент закончился на последнем значении сигнала
        partsSignal.lengthLevels_[i] = lenLevel;
        partsSignal.nFragmentLevels_[i] = iFragment;
        // Выделение памяти для всех фрагментов на уровне
        partsSignal.resizeMain(i, lenLevel); // Время, сигнал и флаги
        partsSignal.resizeInd(i); // Индексы концов
        // Заполнение векторов
        lenLevel = 0; iFragment = 0; isFragment = false;
        for (int j = calculationInd_.first; j <= calculationInd_.second; ++j){
            if ( partsSignal.signal_[j] >= lowBoundLevels_[i] && partsSignal.signal_[j] <= upperBoundLevels_[i] ){
                partsSignal.time_[i][lenLevel] = j + 1; // Время
                partsSignal.data_[i][lenLevel] = partsSignal.signal_[j]; // Ускорение
                isFragment = true;
                ++lenLevel;
            } else if (isFragment){
                partsSignal.flags_[i][lenLevel - 1] = 1; // Флаг конца
                partsSignal.ind_[i][iFragment] = lenLevel - 1; // Индекс конца
                isFragment = false;
                ++iFragment;
            }
        }
        // Если последний фрагмент закончился на последнем значении сигнала
        if (isFragment){
            partsSignal.flags_[i][lenLevel - 1] = 1;
            partsSignal.ind_[i][iFragment] = lenLevel - 1;
        }
    }
}

// Усечение коротких фрагментов
void DivisionDataSignal::truncateLevels(PartsObject & partsObject, int firstLevelInd, int lastLevelInd){
    if (lastLevelInd == -1) lastLevelInd = nLevels_ - 1; // Обработка обратной индексации
    for (int i = firstLevelInd; i <= lastLevelInd; ++i){
        int nFragment = partsObject.nFragmentLevels_[i]; // Число фрагментов на уровне
        int lastIndex = 0; // Индекс конца фрагмента
        int lenFragment = 0; // Длина фрагмента
        int maxLenFragment = 0; // Максимальная длина фрагмента
        // Нахождение максимальной длины фрагмента
        for (int j = 0; j != nFragment; ++j){
            lenFragment = partsObject.ind_[i][j] - lastIndex + 1;
            if (lenFragment > maxLenFragment) maxLenFragment = lenFragment; // Запись максимальной длины
            lastIndex = partsObject.ind_[i][j] + 1; // Запоминаем индекс конца фрагмента
        }
        int limTruncate = qCeil(maxLenFragment * truncatePercent_); // Длина среза
        // Срез фрагментов
        lastIndex = 0;
        int endFixDataIndex = 0; // Индекс конца отфильтрованных данных
        int endFixFragmentIndex = 0; // Индекс конца усеченного фрагмента
        int endFragmentIndex = 0; // Индекс конца фрагмента
        double meanFragment = 0; // Среднее значение фрагмента
        for (int j = 0; j != nFragment; ++j){
            endFragmentIndex = partsObject.ind_[i][j];
            lenFragment = endFragmentIndex - lastIndex + 1;
            if (lenFragment > limTruncate){ // Если длина фрагмента достаточна
                meanFragment = meanVec(partsObject.data_[i], lastIndex, endFragmentIndex); // Среднее на фрагменте
                // Пределы
                auto [minFragment, maxFragment] = minMaxVec(partsObject.data_[i], lastIndex, endFragmentIndex); // Получение минимума и максимума
                minFragment = qAbs(minFragment - meanFragment); // Модуль минимума со сдвигом
                maxFragment = qAbs(maxFragment - meanFragment); // Модуль максимума со сдвигом
                if ( minFragment > maxFragment ) maxFragment = minFragment; // Сравнение по модулю
                if ( maxFragment == 0.0 ) maxFragment = 1.0; // Для нулевых фрагментов
                // Перезапись данных
                for (int k = lastIndex; k <= endFragmentIndex; ++k){
                    partsObject.time_[i][endFixDataIndex] = partsObject.time_[i][k];
                    partsObject.data_[i][endFixDataIndex] = (partsObject.data_[i][k] - meanFragment) / maxFragment;
                    partsObject.flags_[i][endFixDataIndex] = partsObject.flags_[i][k];
                    ++endFixDataIndex;
                }
                partsObject.ind_[i][endFixFragmentIndex] = endFixDataIndex - 1; // Записываем индекс конца усеченного фрагмента
                ++endFixFragmentIndex;
            }
            lastIndex = endFragmentIndex + 1; // Запоминаем индекс конца фрагмента
        }
        partsObject.lengthLevels_[i] = endFixDataIndex; // Длина уровня после усечения
        partsObject.nFragmentLevels_[i] = endFixFragmentIndex; // Число фрагментов после усечения
    }
}

// Вычисление производных
void DivisionDataSignal::derivativeLevels(PartsObject & partsObject, int firstLevelInd, int lastLevelInd){
    if (lastLevelInd == -1) lastLevelInd = nLevels_ - 1; // Обработка обратной индексации
    for (int i = firstLevelInd; i <= lastLevelInd; ++i){ // Цикл по всем уровням
        int nFragment = partsObject.nFragmentLevels_[i]; // Число фрагментов на уровне
        int lenFragment = 0; // Длина текущего фрагмента
        int lastIndex = 0; // Индекс конца фрагмента
        int endFragmentIndex = 0; // Индекс конца фрагмента
        for (int j = 0; j != nFragment; ++j){ // Цикл по всем фрагментам
            endFragmentIndex = partsObject.ind_[i][j];
            lenFragment = endFragmentIndex - lastIndex + 1;
            if (lenFragment != 1){ // Если фрагмент не одиночный
                for (int k = lastIndex; k != endFragmentIndex; ++k)
                    partsObject.derivative_[i][k] = partsObject.data_[i][k + 1] - partsObject.data_[i][k]; // Правая конечная разность
                partsObject.derivative_[i][endFragmentIndex] = partsObject.derivative_[i][endFragmentIndex - 1]; // Копия
            } else
                partsObject.derivative_[i][lastIndex] = 0.0;
            lastIndex = endFragmentIndex + 1;
        }
    }
}

// Склейка по уровням
void DivisionDataSignal::glueLevels(PartsObject & partsObject, int firstLevelInd, int lastLevelInd){
    if (lastLevelInd == -1) lastLevelInd = nLevels_ - 1; // Обработка обратной индексации
    // .. //
}

// Выделение монотонных уровней
void DivisionDataSignal::constructMonotoneLevels(QVector<PartsMonotone *> & vecPartsMonotone, int firstLevelInd, int lastLevelInd){
    if (lastLevelInd == -1) lastLevelInd = nLevels_ - 1; // Обработка обратной индексации
    static double MAX_SEPARATION = 0.05; // Максимальное расхождение границ
    static PartsObject const& baseAccel = vecPartsMonotone[0]->basePartsAccel_; // Базовые части ускорений
    static QVector<QVector<double>> const& baseDisplacementData = vecPartsMonotone[0]->basePartsDisplacement_.data_; // Данные частей перемещений
    for (int i = firstLevelInd; i <= lastLevelInd; ++i){ // По всем уровням
        // Выделяем память с запасом по длине базового уровня
        for (int m = 0; m != 3; ++m)
            vecPartsMonotone[m]->resizeMain(i, baseAccel.lengthLevels_[i]);
        double thresholdSeparate = qAbs(upperBoundLevels_[i] - lowBoundLevels_[i]) * MAX_SEPARATION;
        int lenFragment = 0; // Длина текущего фрагмента
        int lastIndex = 0; // Индекс конца фрагмента
        int endFragmentIndex = 0; // Индекс конца фрагмента
        int nFragment = baseAccel.nFragmentLevels_[i]; // Число фрагментов на уровне
        double difference = 0; // Разница между концами фрагмента
        int indMonotone = 0; // Индекс подходящей монотонной части
        QVector<int> vecLenMonotone = {0, 0, 0}; // Длины частей
        for (int j = 0; j != nFragment; ++j){ // Цикл по всем фрагментам
            endFragmentIndex = baseAccel.ind_[i][j];
            lenFragment = endFragmentIndex - lastIndex + 1;
            difference = baseDisplacementData[i][endFragmentIndex] - baseDisplacementData[i][lastIndex];
            // Оценка монотонности
            if (qAbs(difference) > thresholdSeparate){
                if (difference > 0)      // Возрастающий
                    indMonotone = 0;
                else                     // Убывающий
                    indMonotone = 1;
            } else                       // Нейтральный
                indMonotone = 2;
            // Запись данных
            int & iLenMonotone = vecLenMonotone[indMonotone]; // Длина текущей монотонной части
            PartsMonotone & partMonotone = *vecPartsMonotone[indMonotone];  // Одна из монотонных частей
            for (int k = lastIndex; k <= endFragmentIndex; ++k){
                partMonotone.time_[i][iLenMonotone] = baseAccel.time_[i][k];
                partMonotone.data_[i][iLenMonotone] = baseAccel.data_[i][k];
                partMonotone.flags_[i][iLenMonotone] = baseAccel.flags_[i][k];
                ++iLenMonotone;
            }
            ++partMonotone.nFragmentLevels_[i]; // Увеличиваем число фрагментов для уровня
            lastIndex = endFragmentIndex + 1;
        }
        // Записываем длины монотонных частей
        for (int m = 0; m != 3; ++m){
            int lenLevel = vecLenMonotone[m]; // Длина уровня
            PartsMonotone & partMonotone = *vecPartsMonotone[m];
            partMonotone.lengthLevels_[i] = lenLevel;
            nFragment = partMonotone.nFragmentLevels_[i]; // Число фрагментов в монотонной части
            if (lenLevel == 0) continue; // Если уровень пуст
            partMonotone.resizeInd(i); // Выделение памяти для индексов
            // Индексация фрагментов
            int lastInd = 0;
            for (int k = 0; k != lenLevel; ++k){
                if (partMonotone.flags_[i][k] == 1){
                    partMonotone.ind_[i][lastInd] = k;
                    ++lastInd;
                }
            }
        }
    }
}

// Вызов метода в многопоточном режиме
template <typename T>
void DivisionDataSignal::callMultiThread(T & someObject, void (DivisionDataSignal::*method)(T &, int, int)){
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
        thread[i] = std::thread(method, this, std::ref(someObject), firstLevel, lastLevel - 1);
        firstLevel = lastLevel;
    }
    // Блокируем основной поток до выполнения вызванных
    for (int i = 0; i != nThread; ++i)
        thread[i].join();
}

// Задание индексов расчетных границ
void DivisionDataSignal::setCalculationInd(int lEstimationBound, int rEstimationBound){
    bool isChanged = false; // Флаг изменения границ
    if (rEstimationBound == -1) rEstimationBound = accel_.size(); // Правая граница по умолчанию
    --lEstimationBound; --rEstimationBound; // Сдвиг границ к индексам
    if (lEstimationBound != calculationInd_.first || rEstimationBound != calculationInd_.second) isChanged = true; // Если хотя бы одна граница изменилась
    calculationInd_ = {lEstimationBound, rEstimationBound};
    // Если уровни до этого уже были созданы и границы сменились
    if ( nLevels_ != 0 && isChanged )
        calculate(); // Вызов расчетного метода
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


