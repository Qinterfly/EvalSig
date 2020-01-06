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
    ui->statusBar->showMessage("Сохранение разбиения по уровням завершилось успешно"); // Вывод информационного сообщения
    lastPath_ = levelsWindow_->lastPath(); // Запись последнего пути
}

// Завершение сохранения относительных статистик
void MainWindow::saveAssociatedStatisticsFinished(){
    ui->statusBar->showMessage("Сохранение относительных статистик завершилось успешно"); // Вывод информационного сообщения
    lastPath_= associatedStatisticsWindow_->lastPath();
}

// Завершение сохранения расчетного шаблона
void MainWindow::calculationTemplateFinished(int state){
    switch (state) {
    case 1:
        ui->statusBar->showMessage("Сохранение расчетного шаблона завершилось успешно"); // Вывод информационного сообщения
        break;
    case 2:
        ui->statusBar->showMessage("Загрузка расчетного шаблона завершилась успешно"); // Вывод информационного сообщения
        break;
    }
    lastPath_= calcTemplateWindow_->lastPath();
}

// --------------------------------------------------------------------------------------------------------------
