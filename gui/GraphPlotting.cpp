#include "MainWindow.h"
#include "ui_MainWindow.h"

// ---- Работа с графиками ---------------------------------------------------------------------------------------

// Добавить график
void MainWindow::addGraph(){
    clearDataEstimationsBoundaries(); // Очистка данных графиков расчетных границ
    int plotInd = ui->listFile->count() - 1; // Индекс графика для построения
    QVariant colorSignal = ui->listFile->item(plotInd)->data(Qt::DecorationRole); // Получения цвета графика
    QVector<double> const& YCompare = vecDataSignal_[plotInd].getData(); // Вектор значений
    QVector<double> XCompare(YCompare.size()); // Вектор отсчетов
    for (int i = 0; i != YCompare.size(); ++i)
        XCompare[i] = i + 1;
    ui->comparePlot->addGraph(); // Добавление графика в конец
    ui->comparePlot->graph(SECONDARY_PLOT_IND + plotInd)->setAdaptiveSampling(false); // Отключение сэмплирования отображаемых значений
    ui->comparePlot->graph(SECONDARY_PLOT_IND + plotInd)->setPen( QPen(colorSignal.value<QColor>()) ) ; // Выставление цвета графика
    ui->comparePlot->graph(SECONDARY_PLOT_IND + plotInd)->setData(XCompare, YCompare, true); // Передача отсортированных данных
    bool onlyEnlarge = false; // Опция одностороннего расширения пределов построения
    if (plotInd != 0) onlyEnlarge = true; // В случае multiPlot, подстраиваем под предельные значения
    if ( vecDataSignal_[plotInd].isSpectrum() ){ // В случае, если сигнал спектр, отображаем частоты
        ui->comparePlot->xAxis2->setVisible(true); //
        ui->comparePlot->xAxis2->setRange(0, vecDataSignal_[plotInd].nyquistFrequency());
    }
    ui->comparePlot->rescaleAxes(onlyEnlarge); // Масштабирование осей
    plotEstimationBoundaries(); // Построение графиков расчетных границ
    ui->comparePlot->replot(); // Обновление окна построения
}

// Удалить график
void MainWindow::removeGraph(int deleteInd){
    clearDataEstimationsBoundaries(); // Очистка данных графиков расчетных границ
    ui->comparePlot->removeGraph(SECONDARY_PLOT_IND + deleteInd); // Удаление графика
    bool isExistSpectrum = false; // Флаг наличия графиков спектров
    for (auto dataSignal : vecDataSignal_) // Проверка существования спектров в наборе графиков
        if ( dataSignal.isSpectrum() ) { isExistSpectrum = true; break; }
    if (!isExistSpectrum) // В случае, если спектров не осталось, скрываем частоты
        ui->comparePlot->xAxis2->setVisible(false);
    ui->comparePlot->rescaleAxes(false); // Масштабирование осей
    plotEstimationBoundaries(); // Построение графиков расчетных границ
    ui->comparePlot->replot(); // Обновление окна построения
}

// Обновление графика
void MainWindow::replotGraph(int plotInd){
    QVector<double> const& YCompare = vecDataSignal_[plotInd].getData(); // Вектор значений
    QVector<double> XCompare(YCompare.size()); // Вектор отсчетов
    for (int i = 0; i != YCompare.size(); ++i)
        XCompare[i] = i + 1;
    ui->comparePlot->graph(SECONDARY_PLOT_IND + plotInd)->setData(XCompare, YCompare, true); // Передача отсортированных данных
    if ( vecDataSignal_[plotInd].isSpectrum() ){ // В случае, если сигнал спектр, отображаем частоты
        ui->comparePlot->xAxis2->setVisible(true); //
        ui->comparePlot->xAxis2->setRange(0, vecDataSignal_[plotInd].nyquistFrequency());
    }
    // Выставление пределов
    if (plotInd != 0) // В случае multiPlot, подстраиваем под предельные значения
        ui->comparePlot->rescaleAxes(true); // Масштабирование осей
    else {
        // Экстремумы данных
        double minData = *std::min_element(YCompare.begin(), YCompare.end());
        double maxData = *std::max_element(YCompare.begin(), YCompare.end());
        ui->comparePlot->yAxis->setRange(minData, maxData);
    }
    plotEstimationBoundaries(); // Построение графиков расчетных границ
    ui->comparePlot->replot(); // Обновление окна построения
}

// Построение графиков расчетных границ
void MainWindow::plotEstimationBoundaries(bool isReplot){
    if (ui->comparePlot->graphCount() == SECONDARY_PLOT_IND) return; // При отсутствии сигналов не отображать
    // Граничные значения графика
        // По X
    double leftBound = statSignal_.getEstimationBoundaries().first; // Левая граница
    double rightBound = statSignal_.getEstimationBoundaries().second; // Правая граница
        // По Y
    QCPRange const& tYRange = ui->comparePlot->yAxis->range();
    double minY = tYRange.lower;
    double maxY = tYRange.upper;
    // Построение графиков
    ui->comparePlot->graph(0)->setData({leftBound, rightBound}, {maxY, maxY}, true); // Верхний
    ui->comparePlot->graph(1)->setData({leftBound, rightBound}, {minY, minY}, true); // Нижний
    if ( isReplot ) ui->comparePlot->replot(); // Обновление окна построений
}

// Очистка данных графиков расчетных границ
void MainWindow::clearDataEstimationsBoundaries(){
    for (int plotInd = 0; plotInd != SECONDARY_PLOT_IND; ++plotInd)
        ui->comparePlot->graph(plotInd)->data()->clear();
}

// ------------------------------------------------------------------------------------------------------------------
