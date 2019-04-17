#include <QDebug>
#include <QtMath>
#include "core/NumericalFunctions.h"
#include "include/csaps.h"
#include "include/fftw3.h"
#include "Eigen/Dense"

// ---- Функции обработки временных сигналов -----------------------------------------------------------------------------------------

// Аппроксимация сплайнами
DataSignal approximateSmoothSpline(DataSignal const& dataSignal, double smoothFactor, int nPoint)
{
    // ----------------------------------------------------------------------------------
    // Число точек аппроксимации nPoint:
    // == 0 -- аппроксимация по длине сигнала nDataSignal
    // != 0 -- равномерное разбиение по длине сигнала с сохранением граничных значений
    // ----------------------------------------------------------------------------------
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
        double stepXData = (nDataSignal - 1.0) / double(nPoint);
        for (int i = 0; i != nPoint; ++i)
            xData[i] = 1 + i * stepXData;
    }
    // Формирование выходного временного сигнала
    yData = spline(xData); // Вычисление сплайна на новой сетке
    QVector<double> resYData(nPoint);
    for (int i = 0; i != nPoint; ++i)
        resYData[i] = yData[i];
    return DataSignal(resYData, dataSignal.getProperty());
}

// Аппроксимация по методу наименьших квадратов
DataSignal approximateLeastSquares(DataSignal const& dataSignal, int order, int nPoint){
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
        double stepXData = (nDataSignal - 1.0) / double(nPoint);
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
    return DataSignal(yData, dataSignal.getProperty());
}

// Интегрирование сигнала
QVector<DataSignal> integrate(DataSignal const& dataSignal, int orderIntegral, double smoothFactor = -1){
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
    while (orderIntegral--){
        double sum = 0; // Сумма всех элементов до i-1 -го включительно (по образу)
        // Суммирование
        for (int i = 0; i != nDataSignal; ++i){
            sum += dataImage[i];
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

// Нахождение оконных весовых коэффициентов
QVector<double> computeWeightWindow(QString const& type, int weightWindowWidth){
    bool isChoosed = false; // Индикатор выбора окна
    QVector<double> window; // Весовые коэффициенты
    // Проверка размера окна
    if (weightWindowWidth < 0)
        qDebug() << "Неверный размер весового окна";
    window.resize(weightWindowWidth);
    // Окно Хэмминга
    if (type == "Хэмминга"){
        for (int i = 0; i != weightWindowWidth; ++i)
            window[i] = 0.53836 - ( 0.46164 * qCos(2 * M_PI * i / (weightWindowWidth - 1)) );
        isChoosed = true;
    }
    // Окно Ханна
    else if (type == "Ханна"){
        for (int i = 0; i != weightWindowWidth; ++i)
            window[i] = 0.5 * ( 1 - qCos(2 * M_PI * i / (weightWindowWidth - 1)) );
        isChoosed = true;
    }
    // Окно Блэкмана
    else if (type == "Блэкмана"){
        double alpha = 0.16;
        double a0 = (1.0 - alpha ) / 2.0;
        double a1 = 0.5;
        double a2 = alpha / 2.0;
        for (int i = 0; i != weightWindowWidth; ++i)
            window[i] = a0 - a1 * qCos(2 * M_PI * i / (weightWindowWidth - 1)) + a2 * qCos(4 * M_PI * i / (weightWindowWidth - 1));
        isChoosed = true;
    }
    // Проверка корректности переданного типа окна
    if (!isChoosed)
        qDebug() << "Весовое окно заданного типа не было найдено";
    return window;
}

// Вычисление спектральной мощности сигнала
DataSignal computePowerSpectralDensity(DataSignal const& dataSignal, QString const& typeWindow, int weightWindowWidth, double overlapFactor, double smoothFactor){
    int nInputData = dataSignal.size(); // Длина временных данных
    // Входные-выходные данные
    QVector<double> inputData = dataSignal.getData(); // Временные данные исходного сигнала
    normalizeVec(inputData); // (!) Нормализация исходных данных
    QVector<double> power; // Плотность спектральной мощности
    // Объекты, необходимые для выполнения преобразования
    double * currentData; // Значения сигнала для текущего окна
    fftw_complex * currentFFTResult; // Результат преобразования для текущего окна
    fftw_plan plan; // План для преобразования Фурье
    // Выделение памяти для используемых объектов
    currentData      = (double *) fftw_malloc(sizeof(double) * weightWindowWidth);
    currentFFTResult = (fftw_complex *) fftw_malloc(sizeof( fftw_complex ) * weightWindowWidth);
    // Создание оценочного плана расчета
    plan = fftw_plan_dft_r2c_1d(weightWindowWidth, currentData, currentFFTResult, FFTW_ESTIMATE);
    // Настройка весового окна
    QVector<double> weightWindow = computeWeightWindow(typeWindow, weightWindowWidth);
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
    tProperty.physicalFactor_ = 1; // Безразмерные величины
    tProperty.nCount_ = power.size(); // Длина спектра
    int const spectrumLength = qRound(tProperty.scanPeriod_ / 2.0); // Результирующая длина спектра
//    DataSignal powerSignal = approximateSmoothSpline(DataSignal(power, tProperty), smoothFactor, spectrumLength); // Аппроксимация выходной плотности спектральной мощности
    DataSignal powerSignal = DataSignal(power, tProperty);
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

