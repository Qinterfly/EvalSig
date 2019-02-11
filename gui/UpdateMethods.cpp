#include "MainWindow.h"
#include "ui_MainWindow.h"

// ---- Методы для обновления параметров программы -------------------------------------------------------------

// Обновление строки состояния
void MainWindow::updateStatusBar(){
    int currSignalIndex = ui->listFile->currentRow(); // Индекс выбранного сигнала
    if (currSignalIndex == -1) { clearStatusBar(); return; } // Проверка на пустоту списка сигналов
    int tabInd = ui->showModeWidget->currentIndex(); // Индекс графика
    // Сравнение сигналов
    if (tabInd == 0){
        calcStatusLabel->setText("Среднее: " + QString::number(vecDataSignal_[currSignalIndex].mean()));
    }
    // Цветовые карты
    if (tabInd >= 1 && tabInd <= 5){
        calcStatusLabel->setText("Число окон: " + QString::number(statSignal_.getNumberOfWindows()));
    }
}

// -------------------------------------------------------------------------------------------------------------
