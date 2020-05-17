#include "MainWindow.h"
#include "ui_MainWindow.h"

// ---- Характеристики сигнала ---------------------------------------------------------------------------------

// Очистка обработанных сигналов
void MainWindow::clearSignalCharacteristics(){
    mapSignalCharacteristics_.clear(); // Очистка данных сигналов
    // Удаление данных графиков
    ui->spectrumPlot->clearGraphs(); // Спектра
    ui->integralPlot->clearGraphs(); // Интеграла
    ui->analysisPlot->clearGraphs(); // Анализа
    // Обновление
    ui->spectrumPlot->replot(); // Спектра
    ui->integralPlot->replot(); // Интеграла
    ui->analysisPlot->replot(); // Анализа
}

// Расчета и построение спектра
void MainWindow::calculateAndPlotSpectrum(){

}

// Расчета и построение интеграла
void MainWindow::calculateAndPlotIntegral(){

}

// Расчета и построение анализа
void MainWindow::calculateAndPlotAnalysis(){

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
    // Установка границ для полосового фильтра спектра
    double nyquistFrequency = vecDataSignal_[ui->listFile->currentRow()].nyquistFrequency();
    ui->spinBoxSpectrumLowerFrequency->setMaximum(nyquistFrequency); // Нижняя частота
    ui->spinBoxSpectrumUpperFrequency->setMaximum(nyquistFrequency); // Верхняя частота
}

// Установка фильтрации спектра
void MainWindow::setSpectrumFiltration(){
    bool state = ui->checkBoxSpectrumFiltration->isChecked();
    ui->spinBoxSpectrumLowerFrequency->setEnabled(state); // Нижняя частота
    ui->spinBoxSpectrumUpperFrequency->setEnabled(state); // Верхняя частота
}

// Проверка частот фильтрации спектра
void MainWindow::checkSpectrumFiltrationFrequencies(){
    double lowerFreq = ui->spinBoxSpectrumLowerFrequency->value(); // Нижняя частота
    double upperFreq = ui->spinBoxSpectrumUpperFrequency->value(); // Верхняя частота
    // Проверка превышения нижней частоты значения верхней
    if ( lowerFreq > upperFreq ){
        ui->spinBoxSpectrumLowerFrequency->setValue(upperFreq);
        ui->spinBoxSpectrumUpperFrequency->setValue(lowerFreq);
    }
}

// -------------------------------------------------------------------------------------------------------------
