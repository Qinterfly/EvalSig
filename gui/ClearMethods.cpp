#include "MainWindow.h"
#include "ui_MainWindow.h"

// -- Методы очистки рабочих параметров ---------------------------------------------------------------------------

// Очистка листа со свойствами сигнала
void MainWindow::clearSignalPropertyList(){
    int nTable = ui->tableFileProperty->rowCount(); // Число строк
    for (int i = 0; i != nTable; ++i){
        QTableWidgetItem * item = ui->tableFileProperty->item(i, 1); // Получение элемента таблицы
        item->setText(""); // Очистка элементов таблицы
        if (i == nTable - 1)
            item->setData(Qt::DecorationRole, 0); // Очистка поля выбора цвета
    }
}

// Очистка информационной строки
void MainWindow::clearStatusBar(){
    // Очистка подсказок
    ui->spinBoxTimeWidth->setStatusTip(""); // Ширина временного окна
    ui->spinBoxShiftWindow->setStatusTip(""); // Смещение временного окна
    statusBar()->clearMessage();
    calcStatusLabel->clear();
}

// Очистка проекта
void MainWindow::clearProject(){
    while (ui->listFile->count() != 0) removeSignal(false); // Удаление всех сигналов
    ui->comparePlot->replot(); // Обновление окна построения графиков для сравнения
}

// -----------------------------------------------------------------------------------------------------------------

