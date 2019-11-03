#ifndef MACROSES_H
#define MACROSES_H

#include <QVector>

using TimeRegressionParams = QVector< QPair<double, double> >;             // Распределение параметров регрессии по времени
using ArrayRegressionParams = QVector< QVector< TimeRegressionParams > >;  // Матрица, содержащая параметры регрессии для всех пар сигналов
using ArrayStatCharacters = QVector< QVector< QVector<double> > >;         // Матрица статистических параметров

#endif // MACROSES_H
