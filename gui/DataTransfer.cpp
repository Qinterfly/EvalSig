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
    QString description = "Программа <b>EvalSig</b> предназначена для оценки взаимного влияния сигналов";
    QString version = "v1.7.1";
    QString author = "Copyright © 2019-2021 Павел Лакиза (Qinterfly)";
    QString info = QString("%1<br> Версия программы: %2<br> %3<br>").arg(description).arg(version).arg(author);
    QMessageBox::about(this, "О программе", info);
}

// --------------------------------------------------------------------------------------------------------------
