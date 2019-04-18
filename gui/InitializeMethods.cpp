#include "MainWindow.h"
#include "ui_MainWindow.h"

// ---- Инициализация параметров ---------------------------------------------------------------------------------

// Инициализация листа со свойствами сигнала
void MainWindow::initializeSignalPropertyList(){
    // Выставление параметров доступа
        // Отключение изменений столбца свойств
    int nTable = ui->tableFileProperty->rowCount(); // Число строк
    for (int i = 0; i != nTable; ++i){
        QTableWidgetItem * item = ui->tableFileProperty->item(i, 0); // Получение элемента таблицы
        item->setFlags(item->flags() ^ Qt::ItemIsEditable); // Отключение возможности изменения содержимого колонок
    }
    // Отключение изменений специальных строк
        // Полное имя файла
    QTableWidgetItem * item = ui->tableFileProperty->item(0, 1);
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
        // Количество отсчётов
    item = ui->tableFileProperty->item(11, 1);
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
        // Цвет графика (меняется методом при клике)
    item = ui->tableFileProperty->item(12, 1);
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
}

// Инициализация расчетных параметров
void MainWindow::initializeCalculationParams(){
    // Запись параметров временного окна
    widthTimeWindow_ = ui->spinBoxTimeWidth->value(); // Ширина окна
    shiftWindow_ = ui->spinBoxShiftWindow->value(); // Смещение левой границы временного окна
    statSignal_.setWindowProperty(widthTimeWindow_, shiftWindow_); // Установка параметров окна
    statSignal_.setEstimationBoundaries(ui->spinBoxLeftEstimationBoundary->value(), ui->spinBoxRightEstimationBoundary->value()); // Границы расчета
}

// Инициализация параметров для отображения
void MainWindow::initializeShowParams(){
    setShowParams(); // Номер окна для показа
    // Список цветов
    colorList_.reserve(20); // Выделение буффера на predefined colors
    colorList_ << Qt::black << Qt::red << Qt::darkRed << Qt::green << Qt::darkGreen << Qt::blue;
    colorList_ << Qt::darkBlue << Qt::cyan << Qt::darkCyan << Qt::magenta << Qt::darkMagenta;
    colorList_ << Qt::yellow << Qt::darkYellow << Qt::darkGray;
    // Установка фильтра событий
        // Для главного окна
    this->installEventFilter(this);
        // Для dock виджетов
    ui->dockFileWidget->installEventFilter(this); // Список сигналов
    ui->dockFileWidget->installEventFilter(this); // Свойства сигналов
    // Формирование строки состояния
    calcStatusLabel = new QLabel(""); // Информация о расчете в statusBar
    ui->statusBar->addPermanentWidget(calcStatusLabel);
}

// Инициализация всех графических окон
void MainWindow::initializeAllPlot(){
    // -- comparePlot --
    int const nEstimationBoundaries = 2;
//    ui->comparePlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom); // Установить пользовательские взаимодействия (перетаскивание + масштабирование)
    // Добавление графиков линий, ограничивающих расчетную область
    QBrush brushCompare(QColor(0, 0, 255, 25)); // Полупрозрачная синияя кисть
    for (int plotInd = 0; plotInd != nEstimationBoundaries; ++plotInd){
        ui->comparePlot->addGraph();
        ui->comparePlot->graph(plotInd)->setPen(Qt::NoPen);
        ui->comparePlot->graph(plotInd)->setBrush(brushCompare);
    }
    // -- AllColorMap --
    initializeAllColorMap(); // Инициализация всех цветовых карт
}

// Инициализация цветовых карт
void MainWindow::initializeAllColorMap(){
    int nCMapPlot = 5; // Число цветовых карт
    bool isInterpolate = ui->checkBoxInterpolateColorMap->isChecked(); // Необходимость интерполяции
    // Выделение буфера для хранения объектов
    vecTablePlot_.resize(nCMapPlot); // На графические объекты
    vecColorMap_.resize(nCMapPlot); // На цветовые карты
    vecColorScale_.resize(nCMapPlot); // На цветовые шкалы
    // Заполнение объектов
        // Графические объекты
    vecTablePlot_[0] = ui->anglePlot;
    vecTablePlot_[1] = ui->distancePlot;
    vecTablePlot_[2] = ui->similarityPlot;
    vecTablePlot_[3] = ui->amplitudePlot;
    vecTablePlot_[4] = ui->noisePlot;
    // Цветовые карты и цветовые шкалы
    for (int plotInd = 0; plotInd != nCMapPlot; ++plotInd){
        vecTablePlot_[plotInd]->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom); // Установить пользовательские взаимодействия (перетаскивание + масштабирование)
        vecTablePlot_[plotInd]->axisRect()->setupFullAxesBox(true); // Отображение осей по всем сторонам
        vecColorMap_[plotInd] = new QCPColorMap(vecTablePlot_[plotInd]->xAxis, vecTablePlot_[plotInd]->yAxis); // Инициализация цветовой карты
        vecColorScale_[plotInd] = new QCPColorScale(vecTablePlot_[plotInd]); // Инициализация шкалы
        // Установка свойств
        vecTablePlot_[plotInd]->plotLayout()->addElement(0, 1, vecColorScale_[plotInd]); // Добавление шкалы справа
        vecColorScale_[plotInd]->setType(QCPAxis::atRight); // Метки к шкале справа
        vecColorScale_[plotInd]->axis()->setLabel(""); // Метка шкалы
        vecColorMap_[plotInd]->setColorScale(vecColorScale_[plotInd]); // Ассоциорвать распределение цветов с шкалой
        vecColorMap_[plotInd]->setGradient(QCPColorGradient::gpJet); // Тип градиента цвета для отображения
        vecColorMap_[plotInd]->setInterpolate(isInterpolate); // Выставить состояние интерполяции
    }
}

// -----------------------------------------------------------------------------------------------------------------------
