#include "MainWindow.h"
#include "ui_MainWindow.h"

// ---- Работа с цветовыми картами ------------------------------------------------------------------------------

// Построение цветовых карт
void MainWindow::plotColorMap(){
    int nGrid = statSignal_.size(); // Число точек в сетке
    if (nGrid == 0) return; // Проверка необходимости построения
    ArrayRegressionParams const& tableRegression = statSignal_.getRegressionParams(); // Таблица регресси
    angleColorMap->data()->setSize(nGrid, nGrid); // Размеры
    angleColorMap->data()->setRange(QCPRange(0, nGrid - 1), QCPRange(0, nGrid - 1)); // Координаты элементов
    angleColorMap->setInterpolate(false); // Отключить интерполяцию
    // Построение данных
    for (int i = 0; i != nGrid; ++i){
        for (int j = 0; j != nGrid; ++j){
            double ZAngle = tableRegression[i][j][showWindow_].first; // Срез углового коэффициента
            angleColorMap->data()->setCell(i, j, ZAngle);
        }
    }
    angleColorMap->rescaleDataRange(); // Масштабировать диапазон значений
    // Синхронизация значений на шкалах
    QCPMarginGroup *marginGroup = new QCPMarginGroup(ui->anglePlot);
    ui->anglePlot->axisRect()->setMarginGroup(QCP::msBottom | QCP::msTop, marginGroup);
    angleColorScale->setMarginGroup(QCP::msBottom | QCP::msTop, marginGroup);
    ui->anglePlot->rescaleAxes(); // Масштабирование значений +
    ui->anglePlot->replot();  // Перерисовка
}

// -----------------------------------------------------------------------------------------------------------------
