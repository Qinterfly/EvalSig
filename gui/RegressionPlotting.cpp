#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "gui/QCPColorCurve.h"

// ---- Работа с регрессией ------------------------------------------------------------------------------------

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
    // Настройка легенды
    ui->regressionPlot->legend->setVisible(true);
    char signCoeff = '+';
    if (regressionParams.second < 0) signCoeff = '-';
    QString regressionName = "y = " + QString::number(regressionParams.first) + " * x " + signCoeff + " " + QString::number(qAbs(regressionParams.second));
    // Построение облака точек
    QCPColorCurve * curve = new QCPColorCurve(ui->regressionPlot->xAxis, ui->regressionPlot->yAxis);
    // Создание распределения цветов по точкам
    int nColors = calculationInd.second - calculationInd.first + 1;
    QVector<QColor> colors(nColors);
    QCPColorGradient colorGradient = colorScaleRegression_->gradient();
    QCPRange rangeGradient(0, nColors - 1);
    for (int i = 0; i != nColors; ++i)
        colors[i] = colorGradient.color(i, rangeGradient);
    // Определение стиля и размера кривой
    curve->setLineStyle(QCPCurve::lsNone);
    curve->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 4));
    curve->setData(baseSignal.getData(calculationInd.first, calculationInd.second),
                   compareSignal.getData(calculationInd.first, calculationInd.second), colors);
    curve->removeFromLegend(); // Исключение точек из легенды
    // Построение линейной регрессии
    ui->regressionPlot->addGraph();
    ui->regressionPlot->graph(0)->setAdaptiveSampling(false); // Отключение сэмплирования отображаемых значений
    ui->regressionPlot->graph(0)->setPen(QPen(Qt::blue, 2)) ; // Выставление цвета графика
    ui->regressionPlot->graph(0)->setData(limBaseSignal, valRegression, true); // Передача отсортированных данных
    ui->regressionPlot->graph(0)->setName(regressionName); // Передача имени графика
    // Подпись осей
    ui->regressionPlot->xAxis->setLabel(ui->listFile->item(indexBaseSignal)->text());
    ui->regressionPlot->yAxis->setLabel(ui->listFile->item(indexCompareSignal)->text());
    // Обновление графического окна
    ui->regressionPlot->rescaleAxes(true); // Масштабирование осей
    ui->regressionPlot->replot(); // Обновление окна построения
}

// Очистка линейной регрессии
void MainWindow::clearRegression(){
    // Очистка текстовой информации
    ui->regressionPlot->legend->setVisible(false);
    ui->regressionPlot->xAxis->setLabel("");
    ui->regressionPlot->yAxis->setLabel("");
    ui->regressionPlot->clearItems();
    // Очистка данных
    ui->regressionPlot->clearGraphs();
    ui->regressionPlot->clearPlottables();
    ui->regressionPlot->replot();
}

// Проверка возможности построения линейной регрессии
void MainWindow::updateRegressionState(){
    if (ui->comboBoxRegression->count()){
        ui->pushButtonShowRegression->setEnabled(true);
    } else {
        ui->pushButtonShowRegression->setEnabled(false);
        clearRegression();
    }
}

// -------------------------------------------------------------------------------------------------------------
