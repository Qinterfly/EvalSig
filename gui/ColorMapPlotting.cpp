#include "MainWindow.h"
#include "ui_MainWindow.h"

// ---- Работа с цветовыми картами ------------------------------------------------------------------------------

static const int MAX_TICK_LEN = 12; // Максимальная длина текстовой метки

// Построение цветовых карт
void MainWindow::plotAllColorMap(){
    int nGrid = statSignal_.size(); // Число точек в сетке
    // Проверка необходимости построения
    if (nGrid == 0) {
        clearAllColorMap();
        return;
    }
    for (int plotInd = 0; plotInd != vecColorMap_.size(); ++plotInd){
        vecTablePlot_[plotInd]->clearItems(); // Удаление текстовых меток
        setColorMapData(plotInd, nGrid); // Задание данных по типу отображения
        vecColorMap_[plotInd]->rescaleDataRange(true); // Масштабировать значения на шкале
        // Проверка необходимости отображения значений ячеек
        if ( !ui->checkBoxInterpolateColorMap->isChecked() && vecColorMapType_[plotInd] == STATS ) {
            QCPRange const& valRange = vecColorMap_[plotInd]->data()->valueRange(); // Диапазон значений на карте
            for (int i = 0; i != nGrid; ++i){
                for (int j = 0; j != nGrid; ++j){
                    double cellValue = vecColorMap_[plotInd]->data()->cell(i, j);
                    QColor cellColor = vecColorMap_[plotInd]->gradient().color(cellValue, valRange); // Цвет ячейки
                    // Находим контрастный цвет по яркости [ max(R, G, B) ]
                    QColor textCellColor;
                    if (qMax(qMax(cellColor.red(), cellColor.green()), cellColor.blue()) > 255 / 2)
                        textCellColor = Qt::black;
                    else
                        textCellColor = Qt::white;
                    QCPItemText * cellText = new QCPItemText(vecTablePlot_[plotInd]); // Текст для ячейки
                    cellText->setText(QString::number(cellValue)); // Значение
                    cellText->position->setCoords(i, j); // Позиция
                    cellText->setColor(textCellColor); // Установка инверсированного цвета
                }
            }
        }
        // Синхронизация значений на шкалах
        QCPMarginGroup *marginGroup = new QCPMarginGroup(vecTablePlot_[plotInd]);
        vecTablePlot_[plotInd]->axisRect()->setMarginGroup(QCP::msBottom | QCP::msTop, marginGroup);
        vecColorScale_[plotInd]->setMarginGroup(QCP::msBottom | QCP::msTop, marginGroup);
        vecTablePlot_[plotInd]->rescaleAxes(true); // Масштабирование значений на карте
        vecTablePlot_[plotInd]->xAxis->setRange(-0.5, nGrid - 0.5); // Граничные значения по оси X
        if (vecColorMapType_[plotInd] == STATS) // Статистики
            vecTablePlot_[plotInd]->yAxis->setRange(-0.5, nGrid - 0.5); // Граничные значения по оси Y
        vecTablePlot_[plotInd]->replot();  // Перерисовка
    }
}

// Выставление данных для цветовой карты по индексу
void MainWindow::setColorMapData(int plotInd, int nGrid){
    switch (plotInd){
    // Угловые коэффициенты
    case 0: {
        ArrayRegressionParams const& arrData = statSignal_.getRegressionParams(); // Таблица регрессии
        assignDataForColorMap(arrData, plotInd, nGrid); // Назначение данных по ячейкам
        break;
    }
    // Дистанции
    case 1: {
        ArrayStatCharacters const& arrData = statSignal_.getDistanceScatter();
        assignDataForColorMap(arrData, plotInd, nGrid); // Назначение данных по ячейкам
        break;
    }
    // Коэффициенты подобия
    case 2: {
        ArrayStatCharacters const& arrData = statSignal_.getSimilarityCoeffs();
        assignDataForColorMap(arrData, plotInd, nGrid); // Назначение данных по ячейкам
        break;
    }
    // Амплитуды рассеяния
    case 3:
    {
        ArrayStatCharacters const& arrData = statSignal_.getAmplitudeScatter();
        assignDataForColorMap(arrData, plotInd, nGrid); // Назначение данных по ячейкам
        break;
    }
    // Коэффициент шума
    case 4: {
        ArrayStatCharacters const& arrData = statSignal_.getNoiseCoeffs();
        assignDataForColorMap(arrData, plotInd, nGrid); // Назначение данных по ячейкам
        break;
    }
    // Поверхность спектров
    case 5: {
        assignDataForSpectrumSurface(plotInd, nGrid);
        break;
    }
    // Регрессии
    case 6: {

        break;
    }
    }
}

// Выставление подписей поверхности статистик
void MainWindow::setTextTickerForStats(int plotInd, int nGrid){
    QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
    vecTablePlot_[plotInd]->xAxis->setTicker(textTicker);
    for (int i = 0; i != nGrid; ++i){
        QString currTick = ui->listFile->item(i)->text(); // Текущее имя файла
        if (currTick.size() > MAX_TICK_LEN)
            currTick = currTick.right(MAX_TICK_LEN); // Срез по последним символам
        textTicker->addTick(i, currTick); // Отображения имен файлов
    }
    vecTablePlot_[plotInd]->yAxis->setRangeReversed(true); // Инверсия значений по оси Y
    vecTablePlot_[plotInd]->yAxis->setTickLabelRotation(-90); // Поворот меток
    vecTablePlot_[plotInd]->yAxis->setTicker(textTicker); // Установка меток
}

// Выставление подписей поверхности спектров
void MainWindow::setTextTickerForSpectrum(int plotInd, int nGrid, QVector<bool> mask){
    QSharedPointer<QCPAxisTickerText> textTickerX(new QCPAxisTickerText);
    // Метки по оси абсцисс
    vecTablePlot_[plotInd]->xAxis->setTicker(textTickerX);
    for (int i = 0; i != nGrid; ++i){
        if (!mask[i]) continue; // Пропуск сигналов по маске
        QString currTick = ui->listFile->item(i)->text(); // Текущее имя файла
        if (currTick.size() > MAX_TICK_LEN)
            currTick = currTick.right(MAX_TICK_LEN); // Срез по последним символам
        textTickerX->addTick(i, currTick); // Отображения имен файлов
    }
    // Метки по оси ординат
    QSharedPointer<QCPAxisTickerFixed> tickerY(new QCPAxisTickerFixed);
    vecTablePlot_[plotInd]->yAxis->setTicker(tickerY);
    tickerY->setScaleStrategy(QCPAxisTickerFixed::ssMultiples);
}

// Вставка данных по ячейкам ColorMap
//    ArrayRegressionParams
void MainWindow::assignDataForColorMap(ArrayRegressionParams const& arrData, int plotInd, int nGrid){
    // Задание размеров таблицы
    vecColorMap_[plotInd]->data()->setSize(nGrid, nGrid); // Размеры
    vecColorMap_[plotInd]->data()->setRange(QCPRange(0, nGrid - 1), QCPRange(0, nGrid - 1)); // Диапазон значений
    // Занесение данных в таблицу
    for (int i = 0; i != nGrid; ++i){
        for (int j = 0; j != nGrid; ++j){
            double ZAngle = arrData[i][j][showWindow_].first; // Срез углового коэффициента
            vecColorMap_[plotInd]->data()->setCell(j, i, ZAngle); // (!) С учетом будущего reverse i<->j
        }
    }
    setTextTickerForStats(plotInd, nGrid); // Установка текстовых меток
}
//    ArrayStatCharacters
void MainWindow::assignDataForColorMap(ArrayStatCharacters const& arrData, int plotInd, int nGrid){
    // Задание размеров таблицы
    vecColorMap_[plotInd]->data()->setSize(nGrid, nGrid); // Размеры
    vecColorMap_[plotInd]->data()->setRange(QCPRange(0, nGrid - 1), QCPRange(0, nGrid - 1)); // Диапазон значений
    // Занесение данных в таблицу
    for (int i = 0; i != nGrid; ++i){
        for (int j = 0; j != nGrid; ++j){
            double Z = arrData[i][j][showWindow_]; // Срез углового коэффициента
            vecColorMap_[plotInd]->data()->setCell(j, i, Z); // (!) С учетом будущего reverse i<->j
        }
    }
    setTextTickerForStats(plotInd, nGrid); // Установка текстовых меток
}

// Назначение данных для поверхности спектров
void MainWindow::assignDataForSpectrumSurface(int plotInd, int nGrid){
    int nSpectrum = 0; // Длина спектров
    int iSpectrum = 0; // Число спектров
    double maxFreq = 0.0;   // Максимальная частота
    QVector<bool> mask(vecDataSignal_.size(), false); // Маска
    // Составление маски сигналов для построения
    for (int i = 0; i != nGrid; ++i){
        DataSignal const& dataSignal = vecDataSignal_[i];
        if ( dataSignal.isSpectrum() ){
            if (iSpectrum == 0){ // Эталонный спектр
                nSpectrum = dataSignal.size(); // Длина эталонного спектра
                maxFreq = dataSignal.nyquistFrequency(); // Максимальная частота спектра
                mask[i] = true;
                ++iSpectrum;
            } else { // Проверка сигналов на однородность по сравнению с эталонным
                if (dataSignal.size() == nSpectrum && dataSignal.nyquistFrequency() - maxFreq == 0.0){
                    mask[i] = true;
                    ++iSpectrum;
                }
            }
        }
    }
    if (iSpectrum < 2) return; // Проверка числа спектров в наборе
    // Задание размеров таблицы
    vecColorMap_[plotInd]->data()->setSize(iSpectrum, nSpectrum); // Размеры
    vecColorMap_[plotInd]->data()->setRange(QCPRange(0, iSpectrum - 1), QCPRange(0, maxFreq)); // Диапазон значений
    // Занесение данных в таблицу
    iSpectrum = 0;
    for (int i = 0; i != nGrid; ++i){
        if (!mask[i]) continue; // Пропуск сигналов по маске
        DataSignal const& dataSignal = vecDataSignal_[i];
        for (int j = 0; j != nSpectrum; ++j)
            vecColorMap_[plotInd]->data()->setCell(iSpectrum, j, dataSignal[j]);
        ++iSpectrum;
    }
    setTextTickerForSpectrum(plotInd, nGrid, mask); // Установка текстовых меток по маске
}

// Очистка цветовых карт
void MainWindow::clearAllColorMap(){
    // Очистка осей
    QSharedPointer<QCPAxisTickerText> emptyTextTicker(new QCPAxisTickerText); // Пустые метки на оси
    for (int plotInd = 0; plotInd != vecColorMap_.size(); ++plotInd){
        vecColorMap_[plotInd]->data()->clear(); // Удаление данных
        vecTablePlot_[plotInd]->clearItems(); // Удаление текстовых меток
        vecTablePlot_[plotInd]->xAxis->setTicker(emptyTextTicker); // Очистка оси X
        vecTablePlot_[plotInd]->yAxis->setTicker(emptyTextTicker); // Очистка оси Y
        vecTablePlot_[plotInd]->xAxis2->setTicker(emptyTextTicker); // Очистка оси X2
        vecTablePlot_[plotInd]->yAxis2->setTicker(emptyTextTicker); // Очистка оси Y2
        vecTablePlot_[plotInd]->replot();  // Перерисовка
    }
}

// -----------------------------------------------------------------------------------------------------------------
