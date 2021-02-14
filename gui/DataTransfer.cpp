#include "MainWindow.h"
#include "ui_MainWindow.h"

// ---- Обмен данными между окнами -----------------------------------------------------------------------------

// Завершение сохранения поуровневого разбиения
void MainWindow::saveLevelsFinished(){
    ui->statusBar->showMessage("Сохранение разбиения по уровням завершилось успешно");
    lastPath_ = levelsWindow_->lastPath(); // Запись последнего пути
}

// Завершение сохранения относительных статистик
void MainWindow::saveAssociatedStatisticsFinished(){
    ui->statusBar->showMessage("Сохранение относительных статистик завершилось успешно");
    lastPath_= associatedStatisticsWindow_->lastPath();
}

// Фильтрация сигналов завершена
void MainWindow::filtrationFinished(){
    statSignal_.setEstimationBoundaries(1, statSignal_.maxSizeSignals(), true); // Пересчет статистик
    setStatEstimationBoundaries(statSignal_.getEstimationBoundaries()); // Правка расчетных границ для отображения
    replotAllGraphs();   // Обновление всех графиков
    plotAllColorMap();   // Построение цветовых карт
    setSignalProperty(); // Обновление свойств сигнала
    ui->statusBar->showMessage("Фильтрация сигналов завершена");
    lastPath_ = filterSignalsWindow_->lastPath(); // Запись последнего пути
}

// Информация о программе
void MainWindow::aboutProgram(){
    static QString const description = "Программа <b>EvalSig</b> предназначена для оценки взаимного влияния сигналов";
    static QString const version = "v1.7.2";
    static QString const author = "Copyright © 2019-2021 Павел Лакиза (Qinterfly)";
    static QString const info = QString("%1<br> Версия программы: %2<br> %3<br>").arg(description).arg(version).arg(author);
    QMessageBox::about(this, "О программе", info);
}

// Нажатие клавиши при сравнении сигналов
void MainWindow::keyPressedComparePlot(int key){
    // Установка расчетного диапазона по выбранному участку
    if (key == Qt::Key_F1 && !statSignal_.isEmpty()){
        QCPRange const& rangeX = ui->comparePlot->getXRange();
        ui->spinBoxLeftEstimationBoundary->setValue(qFloor(rangeX.lower));
        ui->spinBoxRightEstimationBoundary->setValue(qFloor(rangeX.upper));
        setStatEstimationBoundaries(false);
        ui->comparePlot->replot();
    }
}

// --------------------------------------------------------------------------------------------------------------
