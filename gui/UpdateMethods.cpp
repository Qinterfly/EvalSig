#include "MainWindow.h"
#include "ui_MainWindow.h"

// ---- Методы для обновления параметров программы -------------------------------------------------------------

// Обновление строки состояния
void MainWindow::updateStatusBar(){
    int currSignalIndex = ui->listFile->currentRow(); // Индекс выбранного сигнала
    if (currSignalIndex == -1) { clearStatusBar(); return; } // Проверка на пустоту списка сигналов
    int tabInd = ui->showModeWidget->currentIndex(); // Индекс графика
    // Сравнение сигналов
    if (tabInd == 0){
        calcStatusLabel->setText("Сред: " + QString::number(statSignal_.getMeanSegment(currSignalIndex), 'g', 3) + " | "
                                    + "Лок. откл: " + QString::number(statSignal_.getLocalDeviationSegment(currSignalIndex), 'g', 3) + " | "
                                    + "Сред. квадр: " + QString::number(statSignal_.getSquareMeanSegment(currSignalIndex), 'g', 3) + " | "
                                    + "Мин/макс: " + QString::number(statSignal_.getMinSegment(currSignalIndex), 'g', 3) + " / "
                                                   + QString::number(statSignal_.getMaxSegment(currSignalIndex), 'g', 3));
    }
    // Цветовые карты
    if (tabInd >= 1 && tabInd <= 5)
        calcStatusLabel->setText(QString("Мин. длина: ") + QString::number(statSignal_.minSizeSignals()) + " | "
                                             + "Число окон: " + QString::number(statSignal_.getNumberOfWindows()));
}

// Проверка возможности построения линейной регрессии
void MainWindow::updateRegressionState(){
    if (ui->comboBoxRegression->count())
        ui->pushButtonShowRegression->setEnabled(true);
    else {
        ui->pushButtonShowRegression->setEnabled(false);
        clearRegression();
    }
}

// Построение линейной регрессии
void MainWindow::plotRegression() {
    clearRegression(); // Очистка рабочего окна
    // Получение индексов сигналов для сравнения
    int indexBaseSignal = ui->listFile->currentRow();
    int indexCompareSignal = ui->comboBoxRegression->currentIndex();
    // Получение ссылок на сигналы
    DataSignal const& baseSignal = vecDataSignal_[indexBaseSignal];
    DataSignal const& compareSignal = vecDataSignal_[indexCompareSignal];
    // Получение индексов построения
    QPair<int, int> calculationInd = statSignal_.getEstimationBoundaries();
    calculationInd = {calculationInd.first - 1, calculationInd.second - 1};
    // Получение границ построения
    QVector<double> limBaseSignal = { statSignal_.getMinSegment(indexBaseSignal), statSignal_.getMaxSegment(indexBaseSignal) };
    // Получение коэффициентов линейной регрессии
    int nWindow = statSignal_.getNumberOfWindows(); // Число окон для вычисления характеристик
    QPair<double, double> const& regressionParams = statSignal_.getRegressionParams()[indexBaseSignal][indexCompareSignal][nWindow]; // (!) Средние коэффициенты
    // Вычисление краевых значений линейной регрессии
    QVector<double> valRegression(2, 0);
    valRegression[0] = regressionParams.first * limBaseSignal[0] + regressionParams.second;
    valRegression[1] = regressionParams.first * limBaseSignal[1] + regressionParams.second;
    // Построение облака точек
    QCPCurve * curve = new QCPCurve(ui->regressionPlot->xAxis, ui->regressionPlot->yAxis);
    curve->setLineStyle(QCPCurve::lsNone);
    curve->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 3));
    curve->setPen(QPen(Qt::red));
    curve->setData(baseSignal.getData(calculationInd.first, calculationInd.second),
                   compareSignal.getData(calculationInd.first, calculationInd.second));
    // Построение линейной регрессии
    ui->regressionPlot->addGraph();
    ui->regressionPlot->graph(0)->setAdaptiveSampling(false); // Отключение сэмплирования отображаемых значений
    ui->regressionPlot->graph(0)->setPen(QPen(Qt::blue, 2)) ; // Выставление цвета графика
    ui->regressionPlot->graph(0)->setData(limBaseSignal, valRegression, true); // Передача отсортированных данных
    // Обновление графического окна
    ui->regressionPlot->rescaleAxes(true); // Масштабирование осей
    ui->regressionPlot->replot(); // Обновление окна построения
}

// Очистка линейной регрессии
void MainWindow::clearRegression(){
    ui->regressionPlot->clearItems();
    ui->regressionPlot->clearGraphs();
    ui->regressionPlot->clearPlottables();
    ui->regressionPlot->replot();
}

// -------------------------------------------------------------------------------------------------------------
