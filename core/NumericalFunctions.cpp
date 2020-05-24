#include <QDebug>
#include <QtMath>
#include "core/NumericalFunctions.h"
#include "include/csaps.h"
#include "include/fftw3.h"
#include "Eigen/Dense"
#include "Eigen_unsupported/Splines"

// ---- Функции обработки временных сигналов -------------------------------------------------------------------

// Аппроксимация сглаживающими сплайнами
DataSignal approximateSmoothSpline(DataSignal const& dataSignal, double smoothFactor, int nPoint, bool isUpdateScanPeriod)
{
    // Число точек аппроксимации nPoint:
    // <= 0 -- аппроксимация по длине сигнала nDataSignal
    // != 0 -- равномерное разбиение по длине сигнала с сохранением граничных значений
    int nDataSignal = dataSignal.size(); // Длина временного сигнала
    if (nPoint <= 0) nPoint = nDataSignal; // По всей длине сигнала
    csaps::DoubleArray xData(nDataSignal), yData(nDataSignal); // Данные сигнала для аппроксимации
    // Исходная сетка сигнала
    for (int i = 0; i != nDataSignal; ++i){
        xData[i] = 1 + i;
        yData[i] = dataSignal[i];
    }
    csaps::UnivariateCubicSmoothingSpline spline(xData, yData, smoothFactor); // Сглаживание сплайном
    // При аппроксимации по заданному набору узлов
    if (nPoint != nDataSignal){
        // Реаллокация памяти для хранения результирующих значений
        xData.resize(nPoint);
        yData.resize(nPoint);
        // Новая координатная сетка
        double stepXData = (nDataSignal - 1.0) / double(nPoint - 1.0);
        for (int i = 0; i != nPoint; ++i)
            xData[i] = 1 + i * stepXData;
    }
    // Формирование выходного временного сигнала
    yData = spline(xData); // Вычисление сплайна на новой сетке
    QVector<double> resYData(nPoint);
    for (int i = 0; i != nPoint; ++i)
        resYData[i] = yData[i];
    // Меняем частоту дискретизации
    PropertyDataSignal property = dataSignal.getProperty();
    if (isUpdateScanPeriod)
        property.scanPeriod_ = property.scanPeriod_ * nDataSignal / nPoint;
    return DataSignal(resYData, property);
}

// Аппроксимация по методу наименьших квадратов
DataSignal approximateLeastSquares(DataSignal const& dataSignal, int order, int nPoint, bool isUpdateScanPeriod){
    int nDataSignal = dataSignal.size(); // Длина сигнала
    if (nPoint <= 0) nPoint = nDataSignal; // Обработка исключения по числу разбиений
    QVector<double> xData(nDataSignal); // Вектор отсчетов
    // Заполнение вектора отсчетов
    for (int i = 0; i != nDataSignal; ++i)
        xData[i] = 1 + i;
    // Формирование СЛАУ
    int nSystem = order + 1; // Размер системы
    Eigen::MatrixXd AMat(nSystem, nSystem); // Матрица невязок
    Eigen::VectorXd XVec(nSystem); // Вектор полиномиальных коэффициентов
    Eigen::VectorXd BVec(nSystem); // Вектор правой части
    double elemSumMat = 0, elemSumVec = 0; // Сумма элементов, входящих в невязку
    double tVal = 0; // Контейнер текущих операций
        // Матрица невязок
    for (int i = 0; i != nSystem; ++i){
        for (int j = 0; j != nSystem; ++j){
            elemSumMat = 0; elemSumVec = 0;
            // Поэлементная сумма
            for (int s = 0; s != nDataSignal; ++s){
                tVal = qPow(xData[s], i); // Элемент, входящий в обе суммы
                elemSumMat += tVal * qPow(xData[s], j); // Сумма в левой части
                elemSumVec += tVal * dataSignal[s]; // Сумма в правой части
            }
            AMat(i, j) = elemSumMat; // Запись суммы в матрицу невязок
            BVec(i) = elemSumVec; // Запись суммы в вектор правой части
        }
    }
    // Получение решения системы
    XVec = AMat.colPivHouseholderQr().solve(BVec);
    // При аппроксимации по заданному набору узлов
    if (nPoint != nDataSignal){
        // Реаллокация памяти для хранения результирующих значений
        xData.resize(nPoint);
        // Новая координатная сетка
        double stepXData = (nDataSignal - 1.0) / double(nPoint - 1);
        for (int i = 0; i != nPoint; ++i)
            xData[i] = 1 + i * stepXData;
    }
    // Вычисление значения полинома на новой сетке
    QVector<double> yData(nPoint);
    for (int i = 0; i != nPoint; ++i){
        tVal = 0;
        // Суммирование значений для заданного узла
        for (int s = 0; s != nSystem; ++s)
            tVal += XVec[s] * qPow(xData[i], s);
        yData[i] = tVal;
    }
    // Меняем частоту дискретизации
    PropertyDataSignal property = dataSignal.getProperty();
    if (isUpdateScanPeriod)
        property.scanPeriod_ = property.scanPeriod_ * nDataSignal / nPoint;
    return DataSignal(yData, property);
}

// Интегрирование сигнала по правилу трапеций с коррекций
QVector<DataSignal> integrateTrapz(DataSignal const& dataSignal, int orderIntegral, double smoothFactor = -1){
    QVector<DataSignal> resVecDataSignal; // Результирующий вектор интегрированного временного сигнала
    if (orderIntegral == 0){ // При нулевом порядке
        resVecDataSignal.push_back(dataSignal);
        return resVecDataSignal;
    }
    if (orderIntegral < 0) orderIntegral = qAbs(orderIntegral); // При отрицательном порядке
    int nDataSignal = dataSignal.size(); // Длина сигнала
    Eigen::VectorXd xData; // Узлы интерполяции корректирующего сплайна
    Eigen::VectorXd corrYData; // Вектор значений корректирующего сплайна
    // Проверка необходимости коррекции
    if (smoothFactor > 0){
        xData.resize(nDataSignal);
        corrYData.resize(nDataSignal);
        for (int i = 0; i != nDataSignal; ++i)
            xData[i] = i + 1;
    }
    QVector<double> resYData(nDataSignal); // Результирующий вектор
    // Интегрирование order-го порядка
    QVector<double> dataImage(dataSignal.getData()); // Образ данных для суммирования
    double timeStep = dataSignal.timeStep(); // Шаг по времени
    while (orderIntegral--){
        double sum = 0; // Сумма по образу
        resYData[0] = 0.0;
        // Суммирование
        for (int i = 1; i != nDataSignal; ++i){
            sum += timeStep * (dataImage[i] + dataImage[i - 1]) / 2.0;
            resYData[i] = sum;
        }
        // Корректировка
        if (smoothFactor > 0){
            // Заполнение корректирующего вектора
            for (int i = 0; i != nDataSignal; ++i)
                corrYData[i] = resYData[i];
            // Создание корректирующего сплайна
            csaps::UnivariateCubicSmoothingSpline corrSpline(xData, corrYData, smoothFactor);
            corrYData = corrSpline(xData); // Вычисление значений в заданных узлах
            // Коррекция по сплайну
            for (int i = 0; i != nDataSignal; ++i)
                resYData[i] -= corrYData[i];
        }
        resVecDataSignal.push_back(DataSignal(resYData, dataSignal.getProperty())); // Запись интеграла order - 1 порядка
        dataImage = resYData; // Перезапись образа
        resYData.fill(0); // Очистка текущего интеграла
    }
    return resVecDataSignal;
}

// Линейная интерполяция сигнала
DataSignal interpolateLinear(DataSignal const& dataSignal, int nDivPoints, bool isInner, bool isUpdateScanPeriod){
    int nDataSignal = dataSignal.size(); // Длина сигнала
    int nResPoints = nDivPoints; // Результирующее число точек
    if ( isInner ) nResPoints = nDivPoints * (nDataSignal - 1) + nDataSignal; // По внутренним точкам
    if (nDataSignal == nResPoints) return dataSignal; // Обработка необходимости интерполяции
    // Формирование новой расчетной сетки
    QVector<double> xData(nResPoints), yData(nResPoints);
    double stepXData = (nDataSignal - 1) / double(nResPoints - 1);
    if ( isInner ) stepXData =  double(nDataSignal - 1) / (nDataSignal - 1) / (nDivPoints + 1); // Шаг с учетом внутренних точек
    for (int i = 0; i != nResPoints; ++i)
        xData[i] = 1 + i * stepXData;
    /* Будем пользоваться тем, что переданный сигнал сформирован на равномерной сетке
    с шагом в 1. В этом случае индекс начала отрезка интерполяции может быть найден без прохода
    по соответсвующей сетке. */
    int leftInd = 0, rightInd; // Индекс начала и конца отрезка интерполяции
    // Проводим интерполяции всех точек отрезка, за исключением границ
    for (int i = 1; i != nResPoints - 1; ++i){
        leftInd = qFloor(xData[i]) - 1;
        rightInd = leftInd + 1;
        yData[i] = (dataSignal[rightInd] - dataSignal[leftInd]) * (xData[i] - rightInd) + dataSignal[leftInd];
    }
    // Копируем начало и конец отрезка
    yData[0] = dataSignal[0];
    yData[nResPoints - 1] = dataSignal[nDataSignal - 1];
    PropertyDataSignal property = dataSignal.getProperty();
    // Меняем частоту дискретизации
    if (isUpdateScanPeriod)
        property.scanPeriod_ = property.scanPeriod_ * nDataSignal / nResPoints;
    return DataSignal(yData, property);
}

// Интерполяция сплайном по общему числу точек
DataSignal interpolateSpline(DataSignal const& dataSignal, QPair<double, double> inputBounds, int nDivPoints, bool isInner, bool isUpdateScanPeriod){
    int nDataSignal = dataSignal.size(); // Длина сигнала
    int nResPoints = nDivPoints; // Результирующее число точек
    if ( isInner ) nResPoints = nDivPoints * (nDataSignal - 1) + nDataSignal; // По внутренним точкам
    if ( nDataSignal == nResPoints || nResPoints == 0 ) return dataSignal;
    Spline spline = getInterpolationSpline(dataSignal, inputBounds); // Вычисление сплайна
    // Заполнение результирующих векторов
    QVector<double> resData(nResPoints);
    double timeStep = (inputBounds.second - inputBounds.first) / (nResPoints - 1); // Шаг по времени по результирующей сетке
    if ( isInner ) timeStep =  (inputBounds.second - inputBounds.first) / (nDataSignal - 1) / (nDivPoints + 1); // Шаг с учетом внутренних точек
    for (int i = 0; i != nResPoints; ++i)
        resData[i] = spline(inputBounds.first + i * timeStep);
    // Меняем частоту дискретизации
    PropertyDataSignal property = dataSignal.getProperty();
    if (isUpdateScanPeriod)
        property.scanPeriod_ = property.scanPeriod_ * nDataSignal / nResPoints;
    return DataSignal(resData, property);
}

// Нахождение оконных весовых коэффициентов
QVector<double> computeWeightWindow(WindowFunction windowFun, int weightWindowWidth){
    bool isChoosed = false; // Индикатор выбора окна
    QVector<double> window; // Весовые коэффициенты
    // Проверка размера окна
    if (weightWindowWidth < 0)
        qDebug() << "Неверный размер весового окна";
    window.resize(weightWindowWidth);
    switch (windowFun){
    case HAMMING: // Окно Хэмминга
        for (int i = 0; i != weightWindowWidth; ++i)
            window[i] = 0.53836 - ( 0.46164 * qCos(2 * M_PI * i / (weightWindowWidth - 1)) );
        isChoosed = true;
        break;
    case HANN: // Окно Ханна
        for (int i = 0; i != weightWindowWidth; ++i)
            window[i] = 0.5 * ( 1 - qCos(2 * M_PI * i / (weightWindowWidth - 1)) );
        isChoosed = true;
        break;
    case BLACKMAN: // Окно Блэкмана
        double alpha = 0.16;
        double a0 = (1.0 - alpha ) / 2.0;
        double a1 = 0.5;
        double a2 = alpha / 2.0;
        for (int i = 0; i != weightWindowWidth; ++i)
            window[i] = a0 - a1 * qCos(2 * M_PI * i / (weightWindowWidth - 1)) + a2 * qCos(4 * M_PI * i / (weightWindowWidth - 1));
        isChoosed = true;
        break;
    }
    // Проверка корректности переданного типа окна
    if (!isChoosed)
        qDebug() << "Весовое окно заданного типа не было найдено";
    return window;
}

// Вычисление спектральной мощности сигнала
DataSignal computePowerSpectralDensity(DataSignal const& dataSignal, WindowFunction windowFun, int weightWindowWidth, double overlapFactor,
                                       int lengthSpectrum, int windowSmoothWidth){
    int nInputData = dataSignal.size(); // Длина временных данных
    // Входные-выходные данные
    QVector<double> inputData = dataSignal.getData(); // Временные данные исходного сигнала
    normalizeVec(inputData, MEAN); // (!) Нормализация исходных данных
    QVector<double> power; // Плотность спектральной мощности
    // Объекты, необходимые для выполнения преобразования
    double * currentData; // Значения сигнала для текущего окна
    fftw_complex * currentFFTResult; // Результат преобразования для текущего окна
    fftw_plan plan; // План для преобразования Фурье
    // Выделение памяти для используемых объектов
    currentData      = (double *) fftw_malloc(sizeof(double) * weightWindowWidth);
    currentFFTResult = (fftw_complex *) fftw_malloc(sizeof( fftw_complex ) * weightWindowWidth);
    // Создание оценочного плана расчета
    plan = fftw_plan_dft_r2c_1d(weightWindowWidth, currentData, currentFFTResult, FFTW_MEASURE);
    // Настройка весового окна
    QVector<double> weightWindow = computeWeightWindow(windowFun, weightWindowWidth);
    int stepWindow = qCeil(weightWindowWidth * (1 - overlapFactor)); // Шаг сдвига окна
    int leftBound = 0, rightBound; // Границы окна
    // Вычислить плотность спектральной мощности по всем окнам
    int outWindowWidth = weightWindowWidth / 2 + 1; // Реальный размер окна с учетом симметрии
    power.resize(outWindowWidth); // Изменение размеров контейнера PSD
    int nWindows = 0; // Число окон
    while (leftBound < nInputData){
        rightBound = leftBound + weightWindowWidth; // Правая граница весового окна
        // Если полное окно не укладывается до конца сигнала
        if (rightBound > nInputData)
            break;
        // Применение оконного преобразования к временным данным
        for (int i = 0; i != weightWindowWidth; ++i)
            currentData[i] = inputData[leftBound + i] * weightWindow[i];
        fftw_execute(plan); // Выполнение дискретного преобразования Фурье
        // Запись полученных значений
        double realVal; // Действительная часть разложения
        double imagVal; // Мнимая часть разложения
        double powVal; // Мощность
        for (int i = 0; i != outWindowWidth; ++i){
            realVal = currentFFTResult[i][0];
            imagVal = currentFFTResult[i][1];
            powVal  = 2 / qPow(outWindowWidth, 2) * (realVal * realVal + imagVal * imagVal);
            if (i == 0) powVal /= 2; // Нормировка центра симметрии
            power[i] += powVal; // Добавление текущего значения мощности в контейнер
        }
        ++nWindows; // Приращение числа окон
        leftBound += stepWindow; // Сдвиг левой границы окна
    }
    // Освобождение ресурсов, использованных для преобразования
    fftw_destroy_plan(plan);
    fftw_free(currentData);
    fftw_free(currentFFTResult);
    // Осреднение спектральной мощности
    for (double & pow : power)
        pow /= nWindows;
    // Формирование выходного сигнала
    PropertyDataSignal tProperty = dataSignal.getProperty(); // Свойства исходного сигнала
    tProperty.physicalFactor_ = 1.0 / nWindows; // Безразмерные величины
    tProperty.isSpectrum = true; // Спектр
    tProperty.characteristic_ += " Спектр";
    DataSignal powerSignal = interpolateLinear(DataSignal(power, tProperty), lengthSpectrum, false, false); // Линейная интерполяция
    if (windowSmoothWidth != 0) // Пропуск сглаживания при нулевой ширине окна
        powerSignal = movingAverageFilter(powerSignal, windowSmoothWidth); // Сглаживание скользящим средним
    return powerSignal;
}

// Корректировка временного сигнала
DataSignal correct(DataSignal const& dataSignal, double smoothFactor){
    int nDataSignal = dataSignal.size(); // Получение размера сигнала
    Eigen::VectorXd xData(nDataSignal); // Узлы интерполяции корректирующего сплайна
    Eigen::VectorXd corrYData(nDataSignal); // Вектор значений корректирующего сплайна
    // Заполнение корректирующего вектора по всем узлам интерполяции
    for (int i = 0; i != nDataSignal; ++i){
        xData[i] = i + 1;
        corrYData[i] = dataSignal[i];
    }
    // Создание корректирующего сплайна
    csaps::UnivariateCubicSmoothingSpline corrSpline(xData, corrYData, smoothFactor);
    corrYData = corrSpline(xData); // Вычисление значений в заданных узлах
    // Коррекция по сплайну
    QVector<double> resYData = dataSignal.getData(); // Получение данных исходного временного сигнала
    for (int i = 0; i != nDataSignal; ++i)
        resYData[i] -= corrYData[i];
    return DataSignal(resYData, dataSignal.getProperty());
}

// Фильтрация сигнала по частотам
DataSignal bandpassFilter(DataSignal const& dataSignal, WindowFunction windowFun, int weightWindowWidth, double overlapFactor, QPair<double, double> const& freqSegment){
    int nInputData = dataSignal.size(); // Длина временных данных
    // Входные-выходные данные
    QVector<double> inputData = dataSignal.getData(); // Временные данные исходного сигнала
    normalizeVec(inputData, MEAN); // (!) Нормализация исходных данных
    QVector<double> resultData; // Результирующие отфильтрованные данные
    // Объекты, необходимые для выполнения преобразования
    double * currentData; // Значения сигнала для текущего окна
    fftw_complex * currentFFTResult; // Результат преобразования для текущего окна
    fftw_plan planForward, planInverse; // Планы для преобразования Фурье
    // Выделение памяти для используемых объектов
    currentData      = (double *) fftw_malloc(sizeof(double) * weightWindowWidth);
    currentFFTResult = (fftw_complex *) fftw_malloc(sizeof( fftw_complex ) * weightWindowWidth);
    // Создание плана расчета
    planForward = fftw_plan_dft_r2c_1d(weightWindowWidth, currentData, currentFFTResult, FFTW_MEASURE); // Прямое преобразование
    planInverse = fftw_plan_dft_c2r_1d(weightWindowWidth, currentFFTResult, currentData, FFTW_MEASURE); // Обратное преобразования
    // Настройка весового окна
    QVector<double> weightWindow = computeWeightWindow(windowFun, weightWindowWidth);
    int stepWindow = qCeil(weightWindowWidth * (1 - overlapFactor)); // Шаг сдвига окна
    int leftBound = 0, rightBound; // Границы окна
    // FFT-IFFT
    int outWindowWidth = weightWindowWidth / 2 + 1; // Реальный размер окна с учетом симметрии
    resultData.resize(nInputData); // Изменение размеров контейнера с результирующими данными
    int nWindows = 0; // Число окон
    double stepFreq = dataSignal.nyquistFrequency() / (outWindowWidth - 1);
    QPair<int, int> indSegment = {qFloor(freqSegment.first / stepFreq), qCeil(freqSegment.second / stepFreq)}; // Индексы границ сегмента
    while (leftBound < nInputData){
        rightBound = leftBound + weightWindowWidth; // Правая граница весового окна
        // Если полное окно не укладывается до конца сигнала
        if (rightBound > nInputData)
            break;
        // Применение оконного преобразования к временным данным
        for (int i = 0; i != weightWindowWidth; ++i)
            currentData[i] = inputData[leftBound + i] * weightWindow[i];
        fftw_execute(planForward); // Выполнение прямого преобразования Фурье
        // Обнуление значений вплоть до первой целевой частоты f_1
        for (int i = 0; i <= indSegment.first; ++i){
            currentFFTResult[i][0] = 0;
            currentFFTResult[i][1] = 0;
        }
        // Обнуление значений после второй целевой частоты f_2
        for (int i = indSegment.second; i < outWindowWidth; ++i){
            currentFFTResult[i][0] = 0;
            currentFFTResult[i][1] = 0;
        }
        fftw_execute(planInverse); // Выполнение обратного преобразования Фурье
        // Применение оконного преобразования к результату обратного преобразования и вставка с перекрытием
        for (int i = 0; i != weightWindowWidth; ++i){
            currentData[i] *= weightWindow[i]; // Домножение на весовое окно
            resultData[leftBound + i] += currentData[i]; // Вставка с перекрытием
        }
        ++nWindows; // Приращение числа окон
        leftBound += stepWindow; // Сдвиг левой границы окна
    }
    // Нормировка результатов расчета
    for (double &res : resultData)
        res /= nWindows;
    // Освобождение ресурсов, использованных для преобразования
    fftw_destroy_plan(planForward);
    fftw_destroy_plan(planInverse);
    fftw_free(currentData);
    fftw_free(currentFFTResult);
    // Формирование выходного сигнала
    PropertyDataSignal tProperty = dataSignal.getProperty(); // Свойства исходного сигнала
    tProperty.physicalFactor_ = 1; // Безразмерные величины
    return DataSignal(resultData, tProperty);
}

// Фильтрация сигнала скользящим средним
DataSignal movingAverageFilter(DataSignal const& dataSignal, int windowWidth){
    int nDataSignal = dataSignal.size();
    if (windowWidth > nDataSignal) windowWidth = nDataSignal; // Проверка ширины окна
    QVector<double> resData(nDataSignal);
    double elemSum = 0; // Поэлементная сумма
    // Проход до ширины окна
    for (int i = 0; i != windowWidth; ++i){
        elemSum += dataSignal[i];
        resData[i] = elemSum / (i + 1);
    }
    // Проход по полным окнам
    for (int i = windowWidth; i != nDataSignal; ++i)
        resData[i] = resData[i - 1] + (dataSignal[i] - dataSignal[i - windowWidth]) / windowWidth;
    return DataSignal(resData, dataSignal.getProperty());
}

// Исключение выбросов из сигнала
DataSignal excludeOutliers(DataSignal const& dataSignal, double limDiff){
    int nDataSignal = dataSignal.size(); // Длина сигнала
    if ( limDiff == 0.0 || nDataSignal < 3 ) return dataSignal;
    QVector<double> resData(nDataSignal); // Результирующие значения сигнала
    // Коррекция центральных значений
    double diff;
    for (int i = 1; i != nDataSignal - 1; ++i){
        diff = abs(dataSignal[i] - dataSignal[i - 1]);
        if ( diff >= limDiff )
            resData[i] = (dataSignal[i + 1] + dataSignal[i - 1]) / 2.0;
        else
            resData[i] = dataSignal[i];
    }
    // Вставка концевых значений
    resData[0] = dataSignal[0];
    resData[nDataSignal - 1] = dataSignal[nDataSignal - 1];
    return DataSignal(resData, dataSignal.getProperty());
}

// Получение интерполяционного сплайна
Spline getInterpolationSpline(DataSignal const& dataSignal, QPair<double, double> inputBounds){
    int nDataSignal = dataSignal.size(); // Длина сигнала
    Eigen::RowVectorXd time(nDataSignal), signal(nDataSignal); // Выделяем память с запасом под исходные и конечные вектора
    // Заполнение исходных векторов
    double timeStep = (inputBounds.second - inputBounds.first) / (nDataSignal - 1);
    for (int i = 0; i != nDataSignal; ++i){
        time[i] = inputBounds.first + i * timeStep;
        signal[i] = dataSignal[i];
    }
    return Spline(time, signal); // Вычисление сплайна
}

// Срез сигнала по времени
DataSignal sliceByTime(DataSignal const& dataSignal, double leftTimeBound, double rightTimeBound){
    int nDataSignal = dataSignal.size(); // Длина сигнала
    int leftInd = dataSignal.convertTimeToCount(leftTimeBound); // Левая индексная граница
    int rightInd = dataSignal.convertTimeToCount(rightTimeBound); // Правая индексная граница
    int nResPoints = rightInd - leftInd + 1; // Результирующее число точек
    if (nDataSignal == nResPoints) return dataSignal;
    QVector<double> resData(nResPoints); // Результирующий сигнал
    for (int i = leftInd; i <= rightInd; ++i)
        resData[i - leftInd] = dataSignal[i];
    return DataSignal(resData, dataSignal.getProperty());
}

// Линейный фильтр
DataSignal linearFilter(DataSignal const& dataSignal, int leftMeanNumber, int rightMeanNumber){
    int nDataSignal = dataSignal.size(); // Длина сигнала
    // Находим осреднение слева
    double leftMeanVal = 0;
    for (int i = 0; i != leftMeanNumber; ++i)
        leftMeanVal += dataSignal[i];
    leftMeanVal /= leftMeanNumber;
    // Находим осреднение справа
    double rightMeanVal = 0;
    for (int i = 0; i != rightMeanNumber; ++i)
        rightMeanVal += dataSignal[nDataSignal - i - 1];
    rightMeanVal /= rightMeanNumber;
    // Строим прямую
    double x1 = leftMeanNumber / 2;
    double x2 = nDataSignal - 1 - rightMeanNumber / 2;
    double deltaX = x2 - x1; // Коэффициент по X
    double deltaY = rightMeanVal - leftMeanVal; // Коэффициент по Y
    QVector<double> resData(nDataSignal);
    for (int i = 0; i != nDataSignal; ++i)
        resData[i] = dataSignal[i] - deltaY / deltaX * (i - x1) - leftMeanVal;
    return DataSignal(resData, dataSignal.getProperty());
}

// Интегрирование сигнала в частотной области
QVector<DataSignal> integrateFreqDomain(DataSignal const& dataSignal, int orderIntegral, WindowFunction windowFun, int weightWindowWidth, double overlapFactor){
    int nInputData = dataSignal.size(); // Длина временных данных
    // Входные-выходные данные
    QVector<double> inputData = dataSignal.getData(); // Временные данные исходного сигнала
    normalizeVec(inputData, MEAN); // (!) Нормализация исходных данных
    QVector<double> resultData; // Результирующие отфильтрованные данные
    // Объекты, необходимые для выполнения преобразования
    double * currentData; // Значения сигнала для текущего окна
    fftw_complex * currentFFTResult; // Результат преобразования для текущего окна
    fftw_plan planForward, planInverse; // Планы для преобразования Фурье
    // Выделение памяти для используемых объектов
    currentData      = (double *) fftw_malloc(sizeof(double) * weightWindowWidth);
    currentFFTResult = (fftw_complex *) fftw_malloc(sizeof( fftw_complex ) * weightWindowWidth);
    // Создание плана расчета
    planForward = fftw_plan_dft_r2c_1d(weightWindowWidth, currentData, currentFFTResult, FFTW_MEASURE); // Прямое преобразование
    planInverse = fftw_plan_dft_c2r_1d(weightWindowWidth, currentFFTResult, currentData, FFTW_MEASURE); // Обратное преобразования
    // Настройка весового окна
    QVector<double> weightWindow = computeWeightWindow(windowFun, weightWindowWidth);
    int stepWindow = qCeil(weightWindowWidth * (1 - overlapFactor)); // Шаг сдвига окна
    int leftBound = 0, rightBound; // Границы окна
    int outWindowWidth = weightWindowWidth / 2 + 1; // Реальный размер окна с учетом симметрии
    int nWindows = 0; // Число окон
    // Расчет частот
    QVector<double> integrationMult(weightWindowWidth);
    double stepFreq = dataSignal.nyquistFrequency() / (outWindowWidth - 1);
    for (int i = 0; i != weightWindowWidth; ++i)
        integrationMult[i] = 1 / (2 * M_PI * stepFreq);
    // Контейнеры результатов
    resultData.resize(nInputData); // Изменение размеров контейнера с результирующими данными
    QVector<DataSignal> vecResultData; // Вектор с результатом
    // Интеграл от весового окна
    double weightWindowIntegral = 0.0;
    for (int i = 1; i != weightWindowWidth; ++i)
        weightWindowIntegral += (weightWindow[i] + weightWindow[i - 1]) / 2.0;
    // FFT-IFFT
    double tempVal = 0.0;
    double timeStep = dataSignal.timeStep();
    while (orderIntegral--) {
        while (leftBound < nInputData){
            rightBound = leftBound + weightWindowWidth; // Правая граница весового окна
            // Если полное окно не укладывается до конца сигнала
            if (rightBound > nInputData)
                break;
            // Применение оконного преобразования к временным данным
            for (int i = 0; i != weightWindowWidth; ++i)
                currentData[i] = inputData[leftBound + i] * weightWindow[i];
            fftw_execute(planForward); // Выполнение прямого преобразования Фурье
            // Выполнение преобразования z_ = z / (i * w)
            for (int i = 0; i < outWindowWidth; ++i){
                // Делим на частоту
                tempVal = integrationMult[i];
                currentFFTResult[i][0] /= tempVal;
                currentFFTResult[i][1] /= tempVal;
                // Делим на мнинмую единицу
                std::swap(currentFFTResult[i][0], currentFFTResult[i][1]);
                currentFFTResult[i][1] *= -1;
            }
            fftw_execute(planInverse); // Выполнение обратного преобразования Фурье
            // Применение оконного преобразования к результату обратного преобразования и вставка с перекрытием
            for (int i = 0; i != weightWindowWidth; ++i){
                currentData[i] *= weightWindow[i]; // Домножение на весовое окно
                resultData[leftBound + i] += currentData[i]; // Вставка с перекрытием
            }
            ++nWindows; // Приращение числа окон
            leftBound += stepWindow; // Сдвиг левой границы окна
        }
        // Нормировка результатов расчета
        for (double &res : resultData)
            res *= timeStep / (2.0 * weightWindowIntegral);
        // Формирование выходного сигнала
        PropertyDataSignal tProperty = dataSignal.getProperty(); // Свойства исходного сигнала
        vecResultData.push_back(DataSignal(resultData, tProperty)); // Вставка результата в контейнер
        // Сохранение результата для следующей итерации
        inputData = resultData;
        normalizeVec(inputData, MEAN); // (!) Нормализация исходных данных
    }
    // Освобождение ресурсов, использованных для преобразования
    fftw_destroy_plan(planForward);
    fftw_destroy_plan(planInverse);
    fftw_free(currentData);
    fftw_free(currentFFTResult);
    return vecResultData;
}

// ---- Вспомогательные ----------------------------------------------------------------------------------------

// Ближайшая предыдущая степень двойки
int previousPow2(int number){
    if (number < 0) number = qAbs(number); // Для отрицательного числа берем модуль
    int pow2 = 0; // Ближайшая степень двойки
    while (number > 1){
        number /= 2; // Целочисленное деление
        ++pow2; // Приращение искомой степени
    }
    return pow2;
}

// Поиск всех делителей
std::set<int> findDivisors(int number, int upperLim){
    if (number <= 0) return std::set<int>();
    if (upperLim == -1) upperLim = number;
    std::set<int> res;
    int sqrRoot = (int) qSqrt(number) + 1;
    int nIter = qMin(sqrRoot, upperLim);
    int k = 0;
    for (int i = 1; i != nIter; ++i){
        if ( number % i == 0 ){
            res.insert(i);
            k = number / i;
            if ( k <= upperLim ) res.insert(k);
        }
    }
    return res;
}

// Вектор с nPoint равномерно распределенных значений [leftBound, rightBound]
QVector<double> linspace(double leftBound, double rightBound, int nPoint){
    QVector<double> linVec(nPoint);
    double step = (rightBound - leftBound) / (nPoint - 1);
    for (int i = 0; i != nPoint; ++i)
        linVec[i] = leftBound + i * step;
    return linVec;
}

// Интерполяция сплайнами
Spline::Spline(Eigen::VectorXd const &vecX, Eigen::VectorXd const &vecY) : xMin_(vecX.minCoeff()), xMax_(vecX.maxCoeff()),
                                                                           order_(std::min<long>(vecX.rows() - 1, 3))
{
    long nVec = vecX.size();
    Eigen::VectorXd vecXS(nVec);
    for (int i = 0; i != nVec; ++i)
        vecXS(i) = scaleValue(vecX(i));
    spline_ = Eigen::SplineFitting<Spline1d>::Interpolate(vecY.transpose(), order_, vecXS.transpose());
}
double Spline::operator()(double x) const {
    return spline_(scaleValue(x))(0);
}
double Spline::scaleValue(double x) const {
    return (x - xMin_) / (xMax_ - xMin_);
}

// Наивный поиск локальных экстремумов
QVector<int> FindPeaksDirect(QVector<double> const& data, int minDistance, std::function<bool(double, double)> compare){
    static int const RESERVE_SIZE = 32;
    int nData = data.size();    // Длина сигнала
    int distance = minDistance; // Число отсчетов до предыдущего экстремума
    double previous = 1.0;      // Предыдущее значение производной
    double current = 0.0;       // Текущее значение производной
    QVector<int> resVec;        // Результирующий вектор корней
    resVec.reserve(RESERVE_SIZE);
    for (int i = 1; i != nData; ++i){
        current = data[i] - data[i - 1];
        // Если знак производной поменялся
        if ( current * previous < 0 && distance >= minDistance && compare(current, 0.0) ){
            distance = 0;
            resVec.push_back(i);
        }
        previous = current;
        ++distance; // Приращение числа отсчетов между экстремумами
    }
    return resVec;
}

// -------------------------------------------------------------------------------------------------------------
