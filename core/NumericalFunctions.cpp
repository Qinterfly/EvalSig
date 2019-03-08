#include "core/NumericalFunctions.h"
#include "include/csaps.h"

// ---- Функции обработки временных сигналов -----------------------------------------------------------------------------------------

// Аппроксимация сплайнами
DataSignal approximateSmoothSpline(DataSignal const& dataSignal, double smoothFactor)
{
    int nDataSignal = dataSignal.size(); // Длина временного сигнала
    double meanVal = dataSignal.mean(); // Среднее временного сигнала
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
    DataSignal resDataSignal(resData, dataSignal.getProperty()); // Новый временной сигнал
    return resDataSignal;
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


