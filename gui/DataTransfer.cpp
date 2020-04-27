#include "MainWindow.h"
#include "ui_MainWindow.h"

// ---- Обмен данными между окнами -----------------------------------------------------------------------------

// Завершение сохранения свойств сигнала
void MainWindow::saveSignalCharacteristicsFinished(){
    ui->statusBar->showMessage("Сохранение характеристик сигнала завершилось успешно"); // Вывод информационного сообщения
    lastPath_ = signalCharacteristicsWindow_->lastPath(); // Запись последнего пути
}

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

// Завершение сохранения расчетного шаблона
void MainWindow::calculationTemplateProcessed(int state){
    switch (state) {
    case CalculationTemplateWindow::Code::Saved:
        ui->statusBar->showMessage("Сохранение расчетного шаблона завершилось успешно"); // Вывод информационного сообщения
        break;
    case CalculationTemplateWindow::Code::Loaded:
        ui->statusBar->showMessage("Загрузка расчетного шаблона завершилась успешно");
        break;
    case CalculationTemplateWindow::Code::StartedApplying:
        ui->statusBar->showMessage("Начато применение расчетного шаблона...");
        break;
    case CalculationTemplateWindow::Code::FinishedApplying:
        ui->statusBar->showMessage("Расчетный шаблон применен успешно");
        break;
    }
    lastPath_= calcTemplateWindow_->lastPath();
}

// Фильтрация сигналов завершена
void MainWindow::filtrationFinished(){
    statSignal_.checkAndRecalculate(); // Пересчет статистик
    setStatEstimationBoundaries();     // Правка расчетных границ
    replotAllGraphs();                 // Обновление всех графиков
    plotAllColorMap();                 // Построение цветовых карт
    ui->statusBar->showMessage("Фильтрация сигналов завершена");
    lastPath_ = filterSignalsWindow_->lastPath(); // Запись последнего пути
}

// Информация о программе
void MainWindow::aboutProgram(){
    QString description = "Программа <b>EvalSig</b> предназначена для оценки взаимного влияния сигналов";
    QString version = "v1.6.4";
    QString author = "Copyright © 2019-2020 Павел Лакиза (Qinterfly)";
    QString info = QString("%1<br> Версия программы: %2<br> %3<br>").arg(description).arg(version).arg(author);
    QMessageBox::about(this, "О программе", info);
}

// --------------------------------------------------------------------------------------------------------------
