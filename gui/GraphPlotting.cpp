#include "MainWindow.h"
#include "ui_mainwindow.h"

// ---- Работа с графиками ---------------------------------------------------------------------------------------

// Добавить график
void MainWindow::addGraph(){
    int plotInd = ui->listFile->count() - 1; // Индекс графика для построения
    QVariant colorSignal = ui->listFile->item(plotInd)->data(Qt::DecorationRole); // Получения цвета графика
    QVector<double> const& YCompare = vecDataSignal_[plotInd].getData(); // Вектор значений
    QVector<double> XCompare(YCompare.size()); // Вектор отсчетов
    for (int i = 0; i != YCompare.size(); ++i)
        XCompare[i] = i + 1;
    ui->comparePlot->addGraph(); // Добавление графика в конец
    ui->comparePlot->graph(plotInd)->setPen( QPen(colorSignal.value<QColor>()) ) ; // Выставление цвета графика
    ui->comparePlot->graph(plotInd)->setData(XCompare, YCompare); // Передача данных
    bool onlyEnlarge = false; // Опция одностороннего расширения пределов построения
    if (plotInd != 0) onlyEnlarge = true; // В случае multiPlot, подстраиваем под предельные значения
    ui->comparePlot->rescaleAxes(onlyEnlarge); // Масштабирование осей
    ui->comparePlot->replot(); // Обновление окна построения
}

// Удалить график
void MainWindow::removeGraph(int deleteInd){
    ui->comparePlot->removeGraph(deleteInd); // Удаление графика
    ui->comparePlot->rescaleAxes(false); // Масштабирование осей
    ui->comparePlot->replot(); // Обновление окна построения
}

// ------------------------------------------------------------------------------------------------------------------
