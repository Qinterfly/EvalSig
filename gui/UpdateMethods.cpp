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
        calcStatusLabel->setText("Сред: " + QString::number(statSignal_.getMeanSegment(currSignalIndex), 'g', 3) + " | "
                                    + "Сред. квадр: " + QString::number(statSignal_.getSquareMeanSegment(currSignalIndex), 'g', 3) + " | "
                                    + "Мин/макс: " + QString::number(statSignal_.getMinSegment(currSignalIndex), 'g', 3) + " / "
                                                   + QString::number(statSignal_.getMaxSegment(currSignalIndex), 'g', 3));
    }
    // Цветовые карты
    if (tabInd >= 1 && tabInd <= 5)
        calcStatusLabel->setText(QString("Мин. длина: ") + QString::number(statSignal_.minSizeSignals()) + " | "
                                             + "Число окон: " + QString::number(statSignal_.getNumberOfWindows()));
}

// -------------------------------------------------------------------------------------------------------------
