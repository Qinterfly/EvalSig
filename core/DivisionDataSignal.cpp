#include <QtMath>
#include <QDebug>
#include <QDir>
#include <thread>
#include <QTextCodec>
#include "DivisionDataSignal.h"
#include "core/NumericalFunctions.h"

// ---- Разбиение базового временного сигнала по уровням согласно опорному -------------------------------------

static const int MAX_THREAD_NUM = 8; // Максимальное число потоков

// Конструктор
DivisionDataSignal::DivisionDataSignal(DataSignal const& base, DataSignal const& support, DataSignal const& approxSupport,
           double levelStep, double overlapFactor, double truncatePercent, double depthGluing, int lEstimationBound, int rEstimationBound)
    : levelStep_(levelStep), truncatePercent_(truncatePercent), depthGluing_(depthGluing),
    base_(base), support_(support), approxSupport_(approxSupport),
    partsBase(base_), partsSupport(support_), // Части базового и опорного
    // Монотонные части
    partsBaseIncrease(partsBase, partsSupport), partsBaseNeutral(partsBase, partsSupport),
    partsBaseDecrease(partsBase, partsSupport)
{
    setOverlapFactor(overlapFactor); // Коэффициент перекрытия
    setCalculationInd(lEstimationBound, rEstimationBound); // Задание расчетных границ
    // Инициализация векторов, определяющих уровни
    lowBoundLevels_ = {0}; upperBoundLevels_ = {0};
    indLevels_ = {0};
    vecPartsBaseMonotone = {&partsBaseIncrease, &partsBaseNeutral, &partsBaseDecrease}; // Запись адресов монотонных частей
}

// Управляющий расчетный метод
void DivisionDataSignal::calculateLevels(){
    createLevels(approxSupport_, calculationInd_, overlapFactor_, levelStep_, lowBoundLevels_, upperBoundLevels_, indLevels_, nLevels_); // Создание расчетных уровней
    // Выделение памяти для частей по уровням
    partsBase.resizeAll(nLevels_);         // Базовые
    partsSupport.resizeAll(nLevels_);      // Опорные
    partsBaseIncrease.resizeAll(nLevels_); // Возрастающие части базовых
    partsBaseNeutral.resizeAll(nLevels_);  // Нейтральные части базовых
    partsBaseDecrease.resizeAll(nLevels_); // Убывающие части базовых
    gluedBase_.resize(nLevels_);           // Склееные базовые
    gluedBaseIncrease_.resize(nLevels_);   // Склееные возрастающие базовые
    gluedBaseDecrease_.resize(nLevels_);   // Склееные убывающие базовые
    gluedBaseNeutral_.resize(nLevels_);    // Склееные нейтральные базовые
    // Получение ссылок на базовые классы
    PartsObject & partsObjectSupport = partsSupport;        // Опорные
    PartsObject & partsObjectBase = partsBase;                  // Базовые
    PartsObject & partsObjectBaseIncrease = partsBaseIncrease;  // Возрастающие базовые
    PartsObject & partsObjectBaseDecrease = partsBaseDecrease;  // Убывающие базовые
    PartsObject & partsObjectBaseNeutral = partsBaseNeutral;    // Нейтральные базовые
    // Выделение частей
    callMultiThread(partsSupport, &DivisionDataSignal::assignLevels); // Опорных
    partsBase.constructByImage(partsSupport);                         // Базовые по образу
    // Усечение уровней
    callMultiThread(partsObjectSupport, &DivisionDataSignal::truncateLevels); // Опорных
    callMultiThread(partsObjectBase, &DivisionDataSignal::truncateLevels);    // Базовых
    // Расчет монотонных частей
    callMultiThread(vecPartsBaseMonotone, &DivisionDataSignal::constructMonotoneLevels);
    // Вычисление производных
    callMultiThread(partsObjectBase, &DivisionDataSignal::derivativeLevels);         // Базовых
    callMultiThread(partsObjectBaseIncrease, &DivisionDataSignal::derivativeLevels); // Возрастающих базовых
    callMultiThread(partsObjectBaseDecrease, &DivisionDataSignal::derivativeLevels); // Убывающих базовых
    callMultiThread(partsObjectBaseNeutral, &DivisionDataSignal::derivativeLevels);  // Нейтральных базовых
    // Вычисление склеек
    callMultiThread({partsObjectBase, gluedBase_}, &DivisionDataSignal::glueLevels);                 // Базовых
    callMultiThread({partsObjectBaseIncrease, gluedBaseIncrease_}, &DivisionDataSignal::glueLevels); // Возрастающих базовых
    callMultiThread({partsObjectBaseDecrease, gluedBaseDecrease_}, &DivisionDataSignal::glueLevels); // Убывающих базовых
    callMultiThread({partsObjectBaseNeutral, gluedBaseNeutral_}, &DivisionDataSignal::glueLevels);   // Нейтральных базовых
    // Запись имен
    for (int i = 0; i != nLevels_; ++i){
        gluedBase_[i].setMeasurePoint("Склееные базовые");
        gluedBaseIncrease_[i].setMeasurePoint("Склееные возрастающие базовые");
        gluedBaseDecrease_[i].setMeasurePoint("Склееные убывающие базовые");
        gluedBaseNeutral_[i].setMeasurePoint("Склееные нейтральные базовые");
    }
}

// Расчет плотности спектральной мощности
void DivisionDataSignal::calculatePowerSpectralDensity(WindowFunction windowFun, double overlapFactorWindow, int lengthSpectrum, int windowSmoothWidth){
    if ( isEmpty() ) return; // Проверка расчета
    static const int MAX_POW = 22; // Максимальная степень двойки, для создания окна
    // Выделение памяти для хранения спектров
    spectrumBase_.resize(nLevels_);
    spectrumBaseIncrease_.resize(nLevels_);
    spectrumBaseDecrease_.resize(nLevels_);
    spectrumBaseNeutral_.resize(nLevels_);
    // Контейнеры
    QVector<QString> const vecSpectrumName = {"Спектры склеенных базовых", "Спектры возрастающих базовых", "Спектры убывающих базовых", "Спектры нейтральных базовых"};
    QVector<QVector<DataSignal> *> const vecSpectrumMonotone = {&spectrumBaseIncrease_, &spectrumBaseDecrease_, &spectrumBaseNeutral_};
    QVector<QVector<DataSignal> const *> const vecMonotone = {&gluedBaseIncrease_, &gluedBaseDecrease_, &gluedBaseNeutral_};
    int nMonotone = vecMonotone.size(); // Число монотонных частей
    // Поиск ширины окна по всем типам частей (минимальная степень двойки)
        // Склейки базовых
    int minBasePow = MAX_POW;
    int tPow = 0;
    for (int i = 0; i != nLevels_; ++i){
        tPow = previousPow2(gluedBase_[i].size());
        if (tPow < minBasePow && tPow != 0)
            minBasePow = tPow;
    }
        // Монотонные склейки
    int minMonotonePow = MAX_POW;
    for (int i = 0; i != nMonotone; ++i){
        QVector<DataSignal> const& elemMonotone = *vecMonotone[i];
        for (int j = 0; j != nLevels_; ++j){
            tPow = previousPow2(elemMonotone[j].size());
            if (tPow < minMonotonePow && tPow != 0)
                minMonotonePow = tPow;
        }
    }
    // Расчет спектров
        // Склеек базовых
    int windowWidthBase = qCeil(qPow(2.0, minBasePow));
    for (int i = 0; i != nLevels_; ++i){
        if (gluedBase_[i].isEmpty()) continue; // Пропуск пустых склеек
        spectrumBase_[i] = computePowerSpectralDensity(gluedBase_[i], windowFun, windowWidthBase, overlapFactorWindow, lengthSpectrum, windowSmoothWidth);
        spectrumBase_[i].setMeasurePoint(vecSpectrumName[0]);
    }
        // Склеек монотонных частей
    int windowWidthMonotone = qCeil(qPow(2.0, minMonotonePow));
    for (int i = 0; i != nMonotone ; ++i){
        QVector<DataSignal> const& elemMonotone = *vecMonotone[i];
        QVector<DataSignal> & elemSpectrumMonotone = *vecSpectrumMonotone[i];
        QString const& tSpectrumName = vecSpectrumName[i + 1];
        for (int j = 0; j != nLevels_; ++j){
            if (elemMonotone[j].isEmpty()) continue; // Пропуск пустых склеек
            elemSpectrumMonotone[j] = computePowerSpectralDensity(elemMonotone[j], windowFun, windowWidthMonotone, overlapFactorWindow, lengthSpectrum, windowSmoothWidth);
            elemSpectrumMonotone[j].setMeasurePoint(tSpectrumName);
        }
    }
}

// Создание расчетных уровней
void DivisionDataSignal::createLevels(DataSignal const& approxSupport, QPair <int, int> const& calculationInd, double overlapFactor, double levelStep,
                                      QVector<double> & lowBoundLevels, QVector<double> & upperBoundLevels, QVector<int> & indLevels, int & nLevels){
    QVector<double> const& data = approxSupport.getData(); // Данные опорных
    auto [min, max] = minMaxVec(data, calculationInd.first, calculationInd.second);
    double mean = meanVec(data, calculationInd.first, calculationInd.second);
    nLevels = 1; // Один уровень гарантирован
    // Обработка единственного уровня
    if (max <= mean + levelStep / 2 && min >= mean - levelStep / 2){
        lowBoundLevels[0] = mean - levelStep / 2;
        upperBoundLevels[0] = mean + levelStep / 2;
        indLevels[0] = 0;
        return;
    }
    // Разбивка по уровням от минимума сигнала
    // Оценка числа уровней
    double lastUpperBound = min + levelStep;
    while (lastUpperBound <= max){
        lastUpperBound += overlapFactor * levelStep; // Сдвиг верхней границы
        ++nLevels;
    }
    // Запись границ уровней
    lowBoundLevels.resize(nLevels);   // Выделяем память для нижних границ
    upperBoundLevels.resize(nLevels); // Выделяем память для верхних границ
    indLevels.resize(nLevels);        // Выделяем память для индексов уровней
    for (int i = 0; i != nLevels; ++i){
        lowBoundLevels[i] = min + i * overlapFactor * levelStep; // Нижние границы
        upperBoundLevels[i] = lowBoundLevels[i] + levelStep; // Нижние границы
    }
    // Нумерация уровней
    int indZeroLevel = qFloor(nLevels / 2.0); // Индекс нулевого уровня
    indLevels[indZeroLevel] = 0;
        // Нижние уровни
    for (int i = indZeroLevel - 1; i >= 0; --i)
        indLevels[i] = i - indZeroLevel;
        // Верхние уровни
    for (int i = indZeroLevel + 1; i != nLevels; ++i)
        indLevels[i] = i - indZeroLevel;
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
void DivisionDataSignal::glueLevels(QPair<PartsObject const&, QVector<DataSignal> &> const& linkageObjects, int firstLevelInd, int lastLevelInd){
    if (lastLevelInd == -1) lastLevelInd = nLevels_ - 1; // Обработка обратной индексации
    static const double MAX_DIFF = 128; // Максимальная разница для критерия
    PartsObject const& partsObject = linkageObjects.first; // Базовые части
    PropertyDataSignal const& propertyObject = partsObject.signal_.getProperty(); // Свойства исходного сигнала
    for (int i = firstLevelInd; i <= lastLevelInd; ++i){ // По всем уровням
        int nFragment = partsObject.nFragmentLevels_[i]; // Число фрагментов на уровне
        if (nFragment < 2) continue; // Пропуск уровня: нечего склеивать
        // Результирующий вектор
        QVector<double> gluedObject(partsObject.lengthLevels_[i]); // Контейнер для склееного сигнала по уровню
        QVector<double> derivativeGluedObject(gluedObject.size()); // Вспомогательный вектор производных
        int lengthGlued = 0; // Длина склееного сигнала
        // Информация для индексации
        int lastIndex = 0; // Индекс начала фрагмента
        int endFragmentIndex = partsObject.ind_[i][0]; // Индекс конца фрагмента
        int lenFragment = endFragmentIndex - lastIndex+ 1; // Длина фрагмента
        QPair<int, int> shiftInd = {0, qCeil(depthGluing_ * lenFragment)}; // Сдвиги границ
        // Вставка левой части
        for (int k = lastIndex; k <= endFragmentIndex; ++k, ++lengthGlued){
            gluedObject[lengthGlued] = partsObject.data_[i][k];
            derivativeGluedObject[lengthGlued] = partsObject.derivative_[i][k];
        }
        // Склейка
        for (int j = 1; j != nFragment; ++j){
            // Запоминаем информацию для следующей итерации
            lastIndex = endFragmentIndex + 1;
            endFragmentIndex = partsObject.ind_[i][j];
            lenFragment = endFragmentIndex - lastIndex + 1;
            shiftInd = {0, qCeil(depthGluing_ * lenFragment)};
            // Выставление левой границы
            if ( lengthGlued > shiftInd.second )
                shiftInd.first = shiftInd.second;
            // Пропуск фрагментов единичной длины
            if ( lenFragment == 1 )
                continue;
            // Подготовка к поиску вариации
            double lSignal = 0; // Значение сигнала на левом конце
            double lDerivative = 0; // Значение производной на левом конце
            double rDerivative = 0; // Значение производной на правом конце
            double diffSignal = 0; // Разница значений сигнала на концах
            double diffDerivative = 0; // Разница производных на концах
            double diffMean = 0; // Среднее между разницами
            double diffFinal = MAX_DIFF; // Финальный критерий различия
            QPair<int, int> bestIndFit = {lengthGlued - 1, lastIndex}; // Индексы лучшей вариации
            // Поиск лучшей вариации
            for (int lInd = lengthGlued - 1; lInd >= lengthGlued - 1 - shiftInd.first; --lInd){ // Уже склеенный сигнал
                lSignal = gluedObject[lInd];
                lDerivative = derivativeGluedObject[lInd];
                for (int rInd = lastIndex; rInd != lastIndex + shiftInd.second; ++rInd){ // Новый фрагмент
                    rDerivative = partsObject.derivative_[i][rInd];
                    if (lDerivative * rDerivative >= 0) {
                        diffSignal = qAbs(lSignal - partsObject.data_[i][rInd]);
                        diffDerivative = qAbs(lDerivative - rDerivative);
                        diffMean = (diffSignal + diffDerivative) / 2.0;
                        if (diffMean < diffFinal){
                            diffFinal = diffMean;
                            bestIndFit = {lInd, rInd};
                        }
                    }
                }
            }
            // Оценка качества поиска
            if ( bestIndFit.second == endFragmentIndex || diffFinal - MAX_DIFF == 0.0)
                continue;
            // Вставка правой части с следующего за подходящим элементов
            lengthGlued = bestIndFit.first + 1;
            for (int k = bestIndFit.second + 1; k <= endFragmentIndex; ++k){
                gluedObject[lengthGlued] = partsObject.data_[i][k];
                derivativeGluedObject[lengthGlued] = partsObject.derivative_[i][k];
                ++lengthGlued;
            }
        }
        gluedObject.resize(lengthGlued); // Реальный размер вектора (всегда меньше выделенного)
        linkageObjects.second[i] = DataSignal(gluedObject, propertyObject); // Определение сигнала
    }
}

// Выделение монотонных уровней
void DivisionDataSignal::constructMonotoneLevels(QVector<PartsMonotone *> & vecPartsMonotone, int firstLevelInd, int lastLevelInd){
    if (lastLevelInd == -1) lastLevelInd = nLevels_ - 1; // Обработка обратной индексации
    static const double MAX_SEPARATION = 0.05; // Максимальное расхождение границ
    PartsObject const& baseObject = vecPartsMonotone[0]->baseObjectParts_; // Базовые части ускорений
    QVector<QVector<double>> const& baseSeparation = vecPartsMonotone[0]->baseSeparationParts_.data_; // Данные для деления
    for (int i = firstLevelInd; i <= lastLevelInd; ++i){ // По всем уровням
        // Выделяем память с запасом по длине базового уровня
        for (int m = 0; m != 3; ++m)
            vecPartsMonotone[m]->resizeMain(i, baseObject.lengthLevels_[i]);
        double thresholdSeparate = qAbs(upperBoundLevels_[i] - lowBoundLevels_[i]) * MAX_SEPARATION;
        int lastIndex = 0; // Индекс конца фрагмента
        int endFragmentIndex = 0; // Индекс конца фрагмента
        int nFragment = baseObject.nFragmentLevels_[i]; // Число фрагментов на уровне
        double difference = 0; // Разница между концами фрагмента
        int indMonotone = 0; // Индекс подходящей монотонной части
        QVector<int> vecLenMonotone = {0, 0, 0}; // Длины частей
        for (int j = 0; j != nFragment; ++j){ // Цикл по всем фрагментам
            endFragmentIndex = baseObject.ind_[i][j];
            difference = baseSeparation[i][endFragmentIndex] - baseSeparation[i][lastIndex];
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
                partMonotone.time_[i][iLenMonotone] = baseObject.time_[i][k];
                partMonotone.data_[i][iLenMonotone] = baseObject.data_[i][k];
                partMonotone.flags_[i][iLenMonotone] = baseObject.flags_[i][k];
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
    if (rEstimationBound == -1) rEstimationBound = base_.size(); // Правая граница по умолчанию
    --lEstimationBound; --rEstimationBound; // Сдвиг границ к индексам
    calculationInd_ = {lEstimationBound, rEstimationBound};
}

// Задание величины смещения уровней
void DivisionDataSignal::setLevelStep(double levelStep){ levelStep_ = levelStep; }
// Задание величины перекрытия уровней
void DivisionDataSignal::setOverlapFactor(double overlapFactor){
    overlapFactor_ = overlapFactor != 0.0 ? overlapFactor : 1;
}
// Задание процента усечения коротких фрагментов
void DivisionDataSignal::setTruncatePercent (double truncatePercent){ truncatePercent_ = truncatePercent; }
// Задание процента глубины склейки правой границы
void DivisionDataSignal::setDepthGluing(double depthGluing){ depthGluing_ = depthGluing; }

// Файловые методы
// Сохранение опорного сигнала
int DivisionDataSignal::writeSupport(QString const& path, QString const& fileName) const {
    return support_.writeDataFile(path, fileName, calculationInd_.first, calculationInd_.second);
}

// Сохранение аппроксимации опорного сигнала
int DivisionDataSignal::writeApproxSupport(QString const& path, QString const& fileName) const {
    return approxSupport_.writeDataFile(path, fileName, calculationInd_.first, calculationInd_.second);
}

// Сохранение всех данных
int DivisionDataSignal::writeAll(QString const& dirName) const{
    int exitStatus = 0; // Код возврата
    exitStatus += writeSupport(dirName, "Опорный.txt");                // Сохранение опорного сигнала
    exitStatus += writeApproxSupport(dirName, "Аппрокс опорный.txt");  // Сохранение аппроксимации опорного сигнала
    exitStatus += writeSpectrum(dirName);                              // Сохранение спектров склеек
    exitStatus += writeGluedParts(dirName);                            // Сохранение склееных частей
    exitStatus += writeInfo(dirName, "Информация об уровнях.txt");     // Сохранение информации об уровнях
    return exitStatus;
}

// Сохранение спектров склеек
int DivisionDataSignal::writeSpectrum(QString const& dirName) const {
    QDir dir(dirName); // Инициализация директории c добавлением разделителя
    if (!dir.exists()) // Проверка существования директории
        dir.mkpath(".");
    QVector<QString> vecSpectrumName = {"Спектры базовых", "Спектры возрастающих базовых", "Спектры убывающих базовых",
                                        "Спектры нейтральных базовых"}; // Директории с именами спектров
    // Создание директорий
    for (QString & spectrumName : vecSpectrumName){
        dir.mkdir(spectrumName);
        spectrumName = dirName + spectrumName + QDir::separator();
    }
    int exitStatus = 0; // Код возврата
    // Базовые
    for (int i = 0; i != nLevels_; ++i){
        if (spectrumBase_[i].isEmpty()) continue;
        exitStatus += spectrumBase_[i].writeDataFile(vecSpectrumName[0], QString::number(i + 1) + ".txt");
    }
    QVector<QVector<DataSignal> const *> const vecSpectrumMonotone = {&spectrumBaseIncrease_, &spectrumBaseDecrease_, &spectrumBaseNeutral_};
    int nMonotone = vecSpectrumMonotone.size(); // Число монотонных частей
    // Монотонные части
    for (int k = 0; k != nMonotone; ++k){
        QVector<DataSignal> const& elemSpectrumMonotone = *vecSpectrumMonotone[k];
        QString const& tSpectrumName = vecSpectrumName[k + 1];
        for (int i = 0; i != nLevels_; ++i){
            if (elemSpectrumMonotone[i].isEmpty()) continue;
            exitStatus += elemSpectrumMonotone[i].writeDataFile(tSpectrumName, QString::number(i + 1) + ".txt");
        }
    }
    return exitStatus;
}

// Сохранение склееных частей
int DivisionDataSignal::writeGluedParts(QString const& dirName) const {
    QDir dir(dirName); // Инициализация директории c добавлением разделителя
    if (!dir.exists()) // Проверка существования директории
        dir.mkpath(".");
    QVector<QString> vecGluedName = {"Склееные базовые", "Склееные возрастающие базовые", "Склееные убывающие базовые",
                                     "Склееные нейтральных базовые"}; // Директории с именами склееных частей
    // Создание директорий
    for (QString & gluedName : vecGluedName){
        dir.mkdir(gluedName);
        gluedName = dirName + gluedName + QDir::separator();
    }
    int exitStatus = 0; // Код возврата
    // Базовые
    for (int i = 0; i != nLevels_; ++i){
        if (gluedBase_[i].isEmpty()) continue;
        exitStatus += gluedBase_[i].writeDataFile(vecGluedName[0], QString::number(i + 1) + ".txt");
    }
    QVector<QVector<DataSignal> const *> const vecGluedMonotone = {&gluedBaseIncrease_, &gluedBaseDecrease_, &gluedBaseNeutral_};
    int nMonotone = vecGluedMonotone.size(); // Число монотонных частей
    // Монотонные части
    for (int k = 0; k != nMonotone; ++k){
        QVector<DataSignal> const& elemGluedMonotone = *vecGluedMonotone[k];
        QString const& tGluedName = vecGluedName[k + 1];
        for (int i = 0; i != nLevels_; ++i){
            if (elemGluedMonotone[i].isEmpty()) continue;
            exitStatus += elemGluedMonotone[i].writeDataFile(tGluedName, QString::number(i + 1) + ".txt");
        }
    }
    return exitStatus;
}

// Сохранение информации об уровнях
int DivisionDataSignal::writeInfo(QString const& dirName, QString const& fileName) const {
    QDir dir(dirName); // Инициализация директории c добавлением разделителя
    if (!dir.exists()) // Проверка существования директории
        dir.mkpath(".");
    QString fileFullPath = dirName + fileName; // Полный путь к файлу
    QFile file(fileFullPath); // Инициализация файла для записи
    if (!checkFile(fileFullPath, "write")){ return -1; } // Обработка ошибок
    file.open(QIODevice::WriteOnly | QIODevice::Text); // Открытие файла для записи
    QTextStream outputStream(&file); // Создание потока для записи
    outputStream.setCodec(QTextCodec::codecForLocale()); // Кодировка по системе
    // Запись информации об уровнях
    outputStream << "Base: " << base_.getName() << endl;
    outputStream << "Support: " << support_.getName() << endl;
    outputStream << "Calculation boundaries: " << QString::number(calculationInd_.first + 1);
    outputStream << " - " << QString::number(calculationInd_.second + 1) << endl;
    outputStream << "Number of levels: " << QString::number(nLevels_) << endl;
    outputStream << "--------------- Control parameters ----------------------" << endl;
    outputStream << "Step: " << QString::number(levelStep_) << endl;
    outputStream << "Overlap factor: " << QString::number(overlapFactor_) << endl;
    outputStream << "Truncate percent: " << QString::number(truncatePercent_) << endl;
    outputStream << "Depth gluing percent: " << QString::number(depthGluing_) << endl;
    outputStream << "-------------- Boundaries of levels ---------------------" << endl;
    for (int i = 0; i != nLevels_; ++i) {
        outputStream << QString::number(indLevels_[i]) << ": " << QString::number(lowBoundLevels_[i]) << ", ";
        outputStream << QString::number(upperBoundLevels_[i]) << endl;
    }
    file.close(); // Закрытие файла
    return 0;
}


// -------------------------------------------------------------------------------------------------------------
