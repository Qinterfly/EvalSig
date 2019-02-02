#include "MainWindow.h"
#include "ui_mainwindow.h"

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
    overlapFactor_ = ui->spinBoxOverlapFactor->value(); // Коэффициент перекрытия
    statSignal_.setWindowProperty(widthTimeWindow_, overlapFactor_); // Установка параметров окна
}

// Инициализация параметров для отображения
void MainWindow::initializeShowParams(){
    setShowParams(); // Номер окна для показа
    // Список цветов
    colorList_.reserve(20); // Выделение буффера на predefined colors
    colorList_ << Qt::black << Qt::red << Qt::darkRed << Qt::green << Qt::darkGreen << Qt::blue;
    colorList_ << Qt::darkBlue << Qt::cyan << Qt::darkCyan << Qt::magenta << Qt::darkMagenta;
    colorList_ << Qt::yellow << Qt::darkYellow << Qt::darkGray;
}

// Инициализация графических окон
void MainWindow::initializePlot(){
    // -- comparePlot --
    ui->comparePlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom); // Установить пользовательские взаимодействия (перетаскивание + масштабирование)
    // -- anglePlot --
    ui->anglePlot->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom); // Установить пользовательские взаимодействия (перетаскивание + масштабирование)
    ui->anglePlot->axisRect()->setupFullAxesBox(true); // Отображение осей по всем сторонам
    angleColorMap = new QCPColorMap(ui->anglePlot->xAxis, ui->anglePlot->yAxis); // Инициализация цветовой карты
    angleColorScale = new QCPColorScale(ui->anglePlot); // Инициализация шкалы
    ui->anglePlot->plotLayout()->addElement(0, 1, angleColorScale); // Добавление шкалы справа
    angleColorScale->setType(QCPAxis::atRight); // Метки к шкале справа
    angleColorMap->setColorScale(angleColorScale); // Ассоциорвать распределение цветов с шкалой
    angleColorScale->axis()->setLabel(""); // Метка шкалы
    angleColorMap->setGradient(QCPColorGradient::gpPolar); // Тип градиента цвета для отображения
}

// -----------------------------------------------------------------------------------------------------------------------
