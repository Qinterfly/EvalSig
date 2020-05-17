#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "core/NumericalFunctions.h"

// ---- Характеристики сигнала ---------------------------------------------------------------------------------

// Очистка обработанных сигналов
void MainWindow::clearSignalCharacteristics(){
    mapSignalCharacteristics_.clear(); // Очистка данных сигналов
    // Удаление данных графиков
    ui->spectrumPlot->clearGraphs(); // Спектра
    ui->integralPlot->clearGraphs(); // Интеграла
    ui->analysisPlot->clearGraphs(); // Анализа
    // Скрытие дополнительных осей
    ui->integralPlot->xAxis2->setVisible(false); // Интеграла
    ui->analysisPlot->xAxis2->setVisible(false); // Анализа
    // Скрытие легенд
    ui->spectrumPlot->legend->setVisible(false); // Спектра
    ui->integralPlot->legend->setVisible(false); // Интеграла
    ui->analysisPlot->legend->setVisible(false); // Анализа
    // Обновление
    ui->spectrumPlot->replot(); // Спектра
    ui->integralPlot->replot(); // Интеграла
    ui->analysisPlot->replot(); // Анализа
}

// Расчета и построение спектра
void MainWindow::calculateAndPlotSpectrum(bool isPlot){
    // Индексы выделенных объектов
    int iSelectedSignal = ui->listFile->currentRow(); // Сигнала
    int iSelectedTab = ui->showModeWidget->currentIndex(); // Вкладки
    // Параметры спектрального разложения
    WindowFunction windowFun = WindowFunction(ui->comboBoxSpectrumWeightWindowType->currentIndex()); // Тип окна (HAMMING, HANN, BLACKMAN)
    int weightWindowWidth = ui->spinBoxSpectrumWeightWindowWidth->value(); // Ширина весового окна
    double overlapFactor = ui->spinBoxSpectrumOverlapFactor->value(); // Коэффициент перекрытия окон
    int lengthSpectrum =  ui->spinBoxSpectrumInterpolation->value(); // Число точек для интерполяции
    int windowSmoothWidth = ui->spinBoxSpectrumSmoothWidth->value(); // Число точек для сглаживания
    // Вычисление спектра сигнала
    mapSignalCharacteristics_.insert(iSelectedTab, computePowerSpectralDensity(vecDataSignal_[iSelectedSignal], windowFun, weightWindowWidth, overlapFactor, lengthSpectrum, windowSmoothWidth));
    // Проверка необходимости построения
    if ( !isPlot )
        return;
    // Формирование данных для построения
    double nyquistFrequency = vecDataSignal_[iSelectedSignal].nyquistFrequency(); // Частота Найквиста, Гц
    QVector<double> const& YData = mapSignalCharacteristics_[iSelectedTab].getData(); // Вектор значений
    int nData = YData.size(); // Длина сигнала
    QVector<double> XData(nData); // Вектор отсчетов
    double freqStep = (nyquistFrequency) / (nData - 1); // Частотный шаг
    for (int i = 0; i != nData; ++i)
        XData[i] = i * freqStep;
     // Построение спектра
    if ( ui->spectrumPlot->graphCount() != 0 )
        ui->spectrumPlot->clearPlottables();
    ui->spectrumPlot->addGraph();
    ui->spectrumPlot->graph()->setAdaptiveSampling(false); // Отключение сэмплирования отображаемых значений
    ui->spectrumPlot->graph()->setPen(QPen(Qt::red)) ; // Выставление цвета графика
    ui->spectrumPlot->graph()->setData(XData, YData, true); // Передача отсортированных данных
    ui->spectrumPlot->graph()->setName(ui->listFile->currentItem()->text()); // Имя графика
    // Обновление графического окна
    ui->spectrumPlot->rescaleAxes(true); // Масштабирование осей
    ui->spectrumPlot->xAxis->setRange(0, nyquistFrequency); // Диапазон частот
    ui->spectrumPlot->legend->setVisible(true); // Отображение легенды
    ui->spectrumPlot->replot(); // Обновление окна построения
}

// Расчета и построение интеграла
void MainWindow::calculateAndPlotIntegral(bool isPlot){
    // Индексы выделенных объектов
    int iSelectedSignal = ui->listFile->currentRow(); // Сигнала
    int iSelectedTab = ui->showModeWidget->currentIndex(); // Вкладки
    // Параметры интегрирования
    int integralOrder = ui->spinBoxIntegralOrder->value(); // Порядок интегрирования
    double correctionFactor = -1; // Корректировочный коэффициент
    // Проверка необходимости коррекции
    if (ui->checkBoxIntegralCorrection->isChecked())
        correctionFactor = ui->spinBoxIntegralCorrectionFactor->value();
    // Вычисление интеграла
    switch ( ui->comboBoxIntegralDomain->currentIndex() ){
        // Во временной области
    case 0:
        mapSignalCharacteristics_.insert(iSelectedTab, integrateTrapz(vecDataSignal_[iSelectedSignal], integralOrder, correctionFactor)[integralOrder - 1]); // Интегрирование сигнала
        break;
        // В частотной области TODO
    case 1:
        mapSignalCharacteristics_.insert(iSelectedTab, integrateTrapz(vecDataSignal_[iSelectedSignal], integralOrder, correctionFactor)[integralOrder - 1]); // Интегрирование сигнала
        break;
    }
    // Проверка необходимости построения
    if ( !isPlot )
        return;
    // Формирование данных для построения
    QVector<double> const& YData = mapSignalCharacteristics_[iSelectedTab].getData(); // Вектор значений
    int nData = YData.size(); // Длина сигнала
    QVector<double> XData(nData); // Вектор отсчетов
    for (int i = 0; i != nData; ++i)
        XData[i] = i + 1;
    // Построение интеграла
    if ( ui->integralPlot->graphCount() != 0 )
        ui->integralPlot->clearPlottables();
    ui->integralPlot->addGraph();
    ui->integralPlot->graph()->setAdaptiveSampling(false); // Отключение сэмплирования отображаемых значений
    ui->integralPlot->graph()->setPen(QPen(Qt::red)) ; // Выставление цвета графика
    ui->integralPlot->graph()->setData(XData, YData, true); // Передача отсортированных данных
    ui->integralPlot->graph()->setName(ui->listFile->currentItem()->text()); // Имя графика
    // Обновление графического окна
    ui->integralPlot->rescaleAxes(true); // Масштабирование осей
    ui->integralPlot->xAxis->setRange(1, nData); // Диапазон отсчетов
    ui->integralPlot->xAxis2->setRange(0, vecDataSignal_[iSelectedSignal].timeDuration()); // Диапазон времени
    ui->integralPlot->xAxis2->setVisible(true); // Отображение вспомогательной оси
    ui->integralPlot->legend->setVisible(true); // Отображение легенды
    ui->integralPlot->replot(); // Обновление окна построения
}

// Расчета и построение анализа
void MainWindow::calculateAndPlotAnalysis(bool isPlot){
    // Индексы выделенных объектов
    int iSelectedSignal = ui->listFile->currentRow(); // Сигнала
    int iSelectedTab = ui->showModeWidget->currentIndex(); // Вкладки
    // Параметры анализа
    double smoothFactor = ui->spinBoxAnalysisSmoothFactor->value(); // Коэффициент сглаживания
    // Анализ
    QString baseName = ui->listFile->currentItem()->text(); // Имя базового сигнала
    QString analysisName = baseName; // Имя анализируемого сигнала
    switch ( ui->comboBoxAnalysisType->currentIndex() ){
        // Аппроксимация
    case 0:
        mapSignalCharacteristics_.insert(iSelectedTab, approximateSmoothSpline(vecDataSignal_[iSelectedSignal], smoothFactor));
        analysisName += "-Аппрокс.";
        break;
        // Коррекция
    case 1:
        mapSignalCharacteristics_.insert(iSelectedTab, correct(vecDataSignal_[iSelectedSignal], smoothFactor));
        analysisName += "-Скоррект.";
        break;
    }
    // Проверка необходимости построения
    if ( !isPlot )
        return;
    // Формирование данных для построения
    QVector<double> const& YData = mapSignalCharacteristics_[iSelectedTab].getData(); // Вектор значений
    int nData = YData.size(); // Длина сигнала
    QVector<double> XData(nData); // Вектор отсчетов
    for (int i = 0; i != nData; ++i)
        XData[i] = i + 1;
    // Очистка предыдущих построений
    if ( ui->analysisPlot->graphCount() != 0 ){
        ui->analysisPlot->clearPlottables();
    }
    // Построение основного сигнала
    ui->analysisPlot->addGraph();
    ui->analysisPlot->graph(0)->setAdaptiveSampling(false);  // Отключение сэмплирования отображаемых значений
    ui->analysisPlot->graph(0)->setPen(QPen(Qt::red));       // Выставление цвета графика
    ui->analysisPlot->graph(0)->setData(XData, vecDataSignal_[iSelectedSignal].getData(), true); // Передача отсортированных данных
    ui->analysisPlot->graph(0)->setName(baseName); // Имя графика
    // Построение сигнала для анализа
    ui->analysisPlot->addGraph();
    ui->analysisPlot->graph(1)->setAdaptiveSampling(false);  // Отключение сэмплирования отображаемых значений
    ui->analysisPlot->graph(1)->setPen(QPen(Qt::blue));      // Выставление цвета графика
    ui->analysisPlot->graph(1)->setData(XData, YData, true); // Передача отсортированных данных
    ui->analysisPlot->graph(1)->setName(analysisName); // Имя графика
    // Обновление графического окна
    ui->analysisPlot->rescaleAxes(true); // Масштабирование осей
    ui->analysisPlot->xAxis->setRange(1, nData); // Диапазон отсчетов
    ui->analysisPlot->xAxis2->setRange(0, vecDataSignal_[iSelectedSignal].timeDuration()); // Диапазон времени
    ui->analysisPlot->xAxis2->setVisible(true); // Отображение вспомогательной оси
    ui->analysisPlot->legend->setVisible(true); // Отображение легенды
    ui->analysisPlot->replot(); // Обновление окна построения
}

// Проверка возможности расчета и сохранения обработанных сигналов
void MainWindow::updateSettingsOfCharacterstics(){
    bool isEmptyProject = ui->listFile->count() == 0; // Пустой проект или нет
    // Кнопки расчета
    QMap<int, QPointer<QPushButton>>::iterator it = mapCalculationButtons_.begin();
    for (; it != mapCalculationButtons_.end(); ++it)
        it.value()->setEnabled(!isEmptyProject);
    // Кнопки сохранения
    if ( isEmptyProject ){
        it = mapSaveButtons_.begin();
        for (; it != mapSaveButtons_.end(); ++it)
            it.value()->setEnabled(false);
        clearSignalCharacteristics();
        return;
    }
}

// Проверка ширины весового окна по указателю
void checkWeightWindowWidth(QPointer<QSpinBox> spinBoxWeightWindowWidth){
    int weightWindowWidth = spinBoxWeightWindowWidth->value(); // Текущая ширина окна
    int weightWindowWidth2 = static_cast<int>(qPow(2, previousPow2(weightWindowWidth))); // Ближайшая ширина окна, кратная двум
    // Установка ширины окна, кратной двум
    if (weightWindowWidth != weightWindowWidth2)
        spinBoxWeightWindowWidth->setValue(weightWindowWidth2);
}

// Проверка ширины весового окна спектра
void MainWindow::checkSpectrumWeightWindowWidth(){
    checkWeightWindowWidth(ui->spinBoxSpectrumWeightWindowWidth);
}

// Проверка ширины весового окна интеграла
void MainWindow::checkIntegralWeightWindowWidth(){
    checkWeightWindowWidth(ui->spinBoxIntegralWeightWindowWidth);
}

// Установка состояния коррекции интеграла
void MainWindow::setEnabledIntegralCorrection(){
    ui->spinBoxIntegralCorrectionFactor->setEnabled(ui->checkBoxIntegralCorrection->isChecked());
}


// -------------------------------------------------------------------------------------------------------------
