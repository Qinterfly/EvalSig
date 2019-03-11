#include <QDebug>
#include <QtMath>
#include "core/NumericalFunctions.h"
#include "include/csaps.h"
#include "include/fftw3.h"

// ---- Функции обработки временных сигналов -----------------------------------------------------------------------------------------

// Аппроксимация сплайнами
DataSignal approximateSmoothSpline(DataSignal const& dataSignal, double smoothFactor)
{
    int nDataSignal = dataSignal.size(); // Длина временного сигнала
    csaps::DoubleArray xData(nDataSignal), yData(nDataSignal); // Данные сигнала для аппроксимации
    // Нормализация сигнала
    for (int i = 0; i != nDataSignal; ++i){
        xData[i] = i + 1;
        yData[i] = dataSignal[i];
    }
    csaps::UnivariateCubicSmoothingSpline spline(xData, yData, smoothFactor); // Сглаживание сплайном
    // Формирование выходного временного сигнала
    yData = spline(xData); // Вычисление сплайна на старой сетке
    QVector<double> resData(nDataSignal);
    for (int i = 0; i != nDataSignal; ++i)
        resData[i] = yData[i];
    return DataSignal(resData, dataSignal.getProperty());
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
            resYData[i] = sum / nDataSignal;
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
        double maxVal = minMaxVec(resYData, [](double a, double b){ return a > b; }); // Поиск максимального значения
        // Нормировка по максимуму
        for (double & val : resYData)
            val /= maxVal;
        resVecDataSignal.push_back(DataSignal(resYData, dataSignal.getProperty())); // Запись интеграла order - 1 порядка
        dataImage = resYData; // Перезапись образа
        resYData.fill(0); // Очистка текущего интеграла
    }
    return resVecDataSignal;
}

// Нахождение оконных весовых коэффициентов
QVector<double> computeWeightWindow(QString const& type, int windowSize){
    bool isChoosed = false; // Индикатор выбора окна
    QVector<double> window; // Весовые коэффициенты
    // Проверка размера окна
    if (windowSize < 0)
        qDebug() << "Неверный размер весового окна";
    window.resize(windowSize);
    // Окно Хэмминга
    if (type == "Hamming"){
        for (int i = 0; i != windowSize; ++i)
            window[i] = 0.53836 - ( 0.46164 * qCos(2 * M_PI * i / (windowSize - 1)) );
        isChoosed = true;
    }
    // Окно Ханна
    else if (type == "Hann"){
        for (int i = 0; i != windowSize; ++i)
            window[i] = 0.5 * ( 1 - qCos(2 * M_PI * i / (windowSize - 1)) );
        isChoosed = true;
    }
    // Окно Блэкмана
    else if (type == "Blackman"){
        double alpha = 0.16;
        double a0 = (1.0 - alpha ) / 2.0;
        double a1 = 0.5;
        double a2 = alpha / 2.0;
        for (int i = 0; i != windowSize; ++i)
            window[i] = a0 - a1 * qCos(2 * M_PI * i / (windowSize - 1)) + a2 * qCos(4 * M_PI * i / (windowSize - 1));
        isChoosed = true;
    }
    // Проверка корректности переданного типа окна
    if (!isChoosed)
        qDebug() << "Весовое окно заданного типа не было найдено";
    return window;
}

// Вычисление спектральной мощности сигнала
DataSignal computePowerSpectralDensity(DataSignal const& dataSignal, QString const& typeWindow, int windowSize, double overlapFactor, double smoothFactor){
    static const int SHIFT_IND_SPECTRUM = 2; // Сдвиг индекса начала спектра
    int nInputData = dataSignal.size(); // Длина временных данных
    // Входные-выходные данные
    QVector<double> const& inputData = dataSignal.getData(); // Временные данные исходного сигнала
    QVector<double> frequency; // Частоты
    QVector<double> power; // Плотность спектральной мощности
    // Объекты, необходимые для выполнения преобразования
    double * currentData; // Значения сигнала для текущего окна
    fftw_complex * currentFFTResult; // Результат преобразования для текущего окна
    fftw_plan plan; // План для преобразования Фурье
    // Выделение памяти для используемых объектов
    currentData      = (double *) fftw_malloc(sizeof(double) * windowSize);
    currentFFTResult = (fftw_complex *) fftw_malloc(sizeof( fftw_complex ) * windowSize);
    // Создание оценочного плана расчета
    plan = fftw_plan_dft_r2c_1d(windowSize, currentData, currentFFTResult, FFTW_ESTIMATE);
    // Настройка весового окна
    QVector<double> weightWindow = computeWeightWindow(typeWindow, windowSize);
    int stepWindow = qCeil(windowSize * (1 - overlapFactor)); // Шаг сдвига окна
    int leftBound = 0, rightBound; // Границы окна
    // Вычислить плотность спектральной мощности по всем окнам
    int outWindowSize = windowSize / 2 + 1; // Реальный размер окна с учетом симметрии
    power.resize(outWindowSize - SHIFT_IND_SPECTRUM); // Изменение размеров контейнера PSD
    int nWindows = 0; // Число окон
    while (leftBound < nInputData){
        rightBound = leftBound + windowSize; // Правая граница весового окна
        // Если полное окно не укладывается до конца сигнала
        if (rightBound > nInputData)
            break;
        // Применение оконного преобразования к временным данным
        for (int i = 0; i != windowSize; ++i)
            currentData[i] = inputData[leftBound + i] * weightWindow[i];
        fftw_execute(plan); // Выполнение дискретного преобразования Фурье
        // Запись полученных значений
        double realVal; // Действительная часть разложения
        double imagVal; // Мнимая часть разложения
        double powVal; // Мощность
        for (int i = SHIFT_IND_SPECTRUM; i != outWindowSize; ++i){
            realVal = currentFFTResult[i][0];
            imagVal = currentFFTResult[i][1];
            powVal  = 2 / qPow(outWindowSize, 2) * (realVal * realVal + imagVal * imagVal);
            if (i == 0) powVal /= 2; // Нормировка центра симметрии
            power[i - SHIFT_IND_SPECTRUM] += powVal; // Добавление текущего значения мощности в контейнер
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
    DataSignal powerSignal = approximateSmoothSpline(DataSignal(power, tProperty), smoothFactor); // Аппроксимация выходной плотности спектральной мощности
    return powerSignal;
}



