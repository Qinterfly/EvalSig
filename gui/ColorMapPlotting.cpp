#include "MainWindow.h"
#include "ui_MainWindow.h"

// ---- Работа с цветовыми картами ------------------------------------------------------------------------------

// Построение цветовых карт
void MainWindow::plotAllColorMap(){
    static const int MAX_TICK_LEN = 12; // Максимальная длина текстовой метки
    int nGrid = statSignal_.size(); // Число точек в сетке
    // Проверка необходимости построения
    if (nGrid == 0) {
        clearAllColorMap();
        return;
    }
    for (int plotInd = 0; plotInd != vecColorMap_.size(); ++plotInd){
        vecTablePlot_[plotInd]->clearItems(); // Удаление текстовых меток
        vecColorMap_[plotInd]->data()->setSize(nGrid, nGrid); // Размеры
        vecColorMap_[plotInd]->data()->setRange(QCPRange(0, nGrid - 1), QCPRange(0, nGrid - 1)); // Диапазон значений
        setColorMapData(plotInd, nGrid); // Задание данных по типу отображения
        vecColorMap_[plotInd]->rescaleDataRange(true); // Масштабировать значения на шкале
        // Подпись меток на оси
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
        // Проверка необходимости отображения значений ячеек
        if (!ui->checkBoxInterpolateColorMap->isChecked()) {
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
        QCPMarginGroup *marginGroup = new QCPMarginGroup(ui->anglePlot);
        vecTablePlot_[plotInd]->axisRect()->setMarginGroup(QCP::msBottom | QCP::msTop, marginGroup);
        vecColorScale_[plotInd]->setMarginGroup(QCP::msBottom | QCP::msTop, marginGroup);
        vecTablePlot_[plotInd]->rescaleAxes(true); // Масштабирование значений на карте
        vecTablePlot_[plotInd]->xAxis->setRange(-0.5, nGrid - 0.5); // Граничные значения по оси X
        vecTablePlot_[plotInd]->yAxis->setRange(-0.5, nGrid - 0.5); // Граничные значения по оси Y
        vecTablePlot_[plotInd]->replot();  // Перерисовка
    }
}

// Выставление данных для цветовой карты по индексу
void MainWindow::setColorMapData(int plotInd, int nGrid){
    // Угловые коэффициенты
    if (plotInd == 0){
        ArrayRegressionParams const& arrData = statSignal_.getRegressionParams(); // Таблица регрессии
        assignDataForColorMap(arrData, plotInd, nGrid); // Назначение данных по ячейкам
        return;
    }
    // Дистанции
    if (plotInd == 1){
        ArrayStatCharacters const& arrData = statSignal_.getDistanceScatter();
        assignDataForColorMap(arrData, plotInd, nGrid); // Назначение данных по ячейкам
        return;
    }
    // Коэффициенты подобия
    if (plotInd == 2){
        ArrayStatCharacters const& arrData = statSignal_.getSimilarityCoeffs();
        assignDataForColorMap(arrData, plotInd, nGrid); // Назначение данных по ячейкам
        return;
    }
    // Амплитуды рассеяния
    if (plotInd == 3){
        ArrayStatCharacters const& arrData = statSignal_.getAmplitudeScatter();
        assignDataForColorMap(arrData, plotInd, nGrid); // Назначение данных по ячейкам
        return;
    }
    // Коэффициент шума
    if (plotInd == 4){
        ArrayStatCharacters const& arrData = statSignal_.getNoiseCoeffs();
        assignDataForColorMap(arrData, plotInd, nGrid); // Назначение данных по ячейкам
        return;
    }
}

// Вставка данных по ячейкам ColorMap
//    ArrayRegressionParams
void MainWindow::assignDataForColorMap(ArrayRegressionParams const& arrData, int plotInd, int nGrid){
    for (int i = 0; i != nGrid; ++i){
        for (int j = 0; j != nGrid; ++j){
            double ZAngle = arrData[i][j][showWindow_].first; // Срез углового коэффициента
            vecColorMap_[plotInd]->data()->setCell(j, i, ZAngle); // (!) С учетом будущего reverse i<->j
        }
    }
}
//    ArrayStatCharacters
void MainWindow::assignDataForColorMap(ArrayStatCharacters const& arrData, int plotInd, int nGrid){
    for (int i = 0; i != nGrid; ++i){
        for (int j = 0; j != nGrid; ++j){
            double Z = arrData[i][j][showWindow_]; // Срез углового коэффициента
            vecColorMap_[plotInd]->data()->setCell(j, i, Z); // (!) С учетом будущего reverse i<->j
        }
    }
}

// Очистка цветовых карт
void MainWindow::clearAllColorMap(){
    // Очистка осей
    QSharedPointer<QCPAxisTickerText> emptyTextTicker(new QCPAxisTickerText); // Пустые метки на оси
    for (int plotInd = 0; plotInd != vecColorMap_.size(); ++plotInd){
        vecTablePlot_[plotInd]->clearItems(); // Удаление текстовых меток
        vecTablePlot_[plotInd]->xAxis->setTicker(emptyTextTicker); // Очистка оси X
        vecTablePlot_[plotInd]->yAxis->setTicker(emptyTextTicker); // Очистка оси Y
        vecTablePlot_[plotInd]->xAxis2->setTicker(emptyTextTicker); // Очистка оси X2
        vecTablePlot_[plotInd]->yAxis2->setTicker(emptyTextTicker); // Очистка оси Y2
        vecTablePlot_[plotInd]->replot();  // Перерисовка
    }
}

// -----------------------------------------------------------------------------------------------------------------
