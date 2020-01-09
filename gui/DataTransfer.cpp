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

// --------------------------------------------------------------------------------------------------------------
