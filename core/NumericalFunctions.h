#ifndef SIGNALPROCESSING_H
#define SIGNALPROCESSING_H

#include "core/DataSignal.h"
#include "Eigen_unsupported/Splines"

using Spline1d = Eigen::Spline<double, 1>;

enum WindowFunction{ HAMMING, HANN, BLACKMAN }; // Типы весовых окон для преобразования Фурье

// Функции обработки временных сигналов
    // Аппроксимация
DataSignal approximateSmoothSpline(DataSignal const& dataSignal, double smoothFactor, int nPoint = -1); // Аппроксимация сплайнами
DataSignal approximateLeastSquares(DataSignal const& dataSignal, int order, int nPoint = -1); // Аппроксимация по методу наименьших квадратов
    // Интегрирование
QVector<DataSignal> integrate(DataSignal const& dataSignal, int orderIntegral, double smoothFactor); // Интегрирование
    // Интерполяция
DataSignal interpolateLinear(DataSignal const& dataSignal, int nPoint); // Линейная интерполяция сигнала
DataSignal interpolateSpline(DataSignal const& dataSignal, QPair<double, double> inputBounds, int nResPoints); // Интерполяция сплайном
    // Вычисление плотности спектральной мощности
DataSignal computePowerSpectralDensity(DataSignal const& dataSignal, WindowFunction windowFun, int widthWindow, double overlapFactor,
                                       int lengthSpectrum, int windowSmoothWidth); // Вычисление спектральной мощности сигнала
    // Фильтрация
DataSignal bandpassFilter(DataSignal const& dataSignal, WindowFunction windowFun, int widthWindow, double overlapFactor, QPair<double, double> const& freqSegment); // Фильтрация сигнала по частотам
DataSignal movingAverageFilter(DataSignal const& dataSignal, int windowLength); // Фильтрация сигнала скользящим средним
    // Корректировка
DataSignal correct(DataSignal const& dataSignal, double smoothFactor); // Корректировка временного сигнала

// Вспомогательные
int previousPow2(int number); // Ближайшая предыдущая степень двойки
QVector<double> linspace(double leftBound, double rightBound, int nPoint); // Вектор с nPoint равномерно распределенных значений [leftBound, rightBound]
    // Интерполяция сплайнами
struct Spline {
public:
    Spline(Eigen::VectorXd const &vecX, Eigen::VectorXd const &vecY);
    Spline(Spline const&) = delete;
    ~Spline() = default;
    double operator()(double x) const; // Получение значение сплайна в заданной точке на отрезке [xMin_, xMax_]
    long order() const { return order_; } // Порядок сплайна
private:
    double scaleValue(double x) const; // Нормировка значений аргумента
private:
    const double xMin_; // Нижний предел интерполяции
    const double xMax_; // Верхний предел интерполяции
    const long order_; // Порядок сплайна
    Spline1d spline_; // Сплайн
};



#endif // SIGNALPROCESSING_H
