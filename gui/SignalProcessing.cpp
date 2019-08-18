#include "MainWindow.h"
#include "ui_MainWindow.h"

// ----  Чтение, запись и удаление сигналов ------------------------------------------------------------------------

// Добавить сигнал
void MainWindow::addSignal(){
    // Организация диалога с пользователем
    QStringList listFullFilePath = QFileDialog::getOpenFileNames(this, "Выберите один или несколько файлов с временными сигналами", lastPath_, "Text files (*.txt)");
    if (listFullFilePath.isEmpty()) return;
    int indFile = 0; // Номер файла в обработке
    for ( QString const& fullFilePath : listFullFilePath){
        int exitStatus = 0; // Код возврата
        ++indFile; // Инкремент номера обрабатываемого файла
        QFileInfo infoName(fullFilePath); // Создание информационного объекта
        QString fileName = infoName.fileName(); // Имя файла
        if (indFile == 1) // Выставление релевантного пути по первому файлу
            lastPath_ = infoName.absolutePath() + QDir::separator(); // Путь к файлу ( + запись в последний выбранный)
        // Добавление сигнала в контейнеры
        exitStatus = statSignal_.addSignal(DataSignal(lastPath_, fileName)); // Расчет статистик + пополнение вектора сигналов
        if (exitStatus == 0){ // При успешном добавлении в контейнер
            QListWidgetItem * item = new QListWidgetItem(infoName.baseName()); // Занесение в список имени без расширения
            int randColorInd = QRandomGenerator::global()->bounded(colorList_.size()); // Случайный цвет из контейнера цветов
            item->setData(Qt::DecorationRole, colorList_[randColorInd]); // Задание цвета для сигнала
            ui->listFile->insertItem(ui->listFile->count(), item); // Запись элемента в список
            setStatEstimationBoundaries(statSignal_.getEstimationBoundaries()); // Правка расчетных границ
            setBoundariesShowParams(); // Выставление границ
            addGraph(); // Добавления сигнала для отрисовки
            statusBar()->showMessage("Сигнал " + fileName + " успешно прочитан"); // Вывод сообщения в statusBar
        }
    }
    plotAllColorMap(); // Построение цветовых карт
    // Установка свойств по последнему сигналу
    int nItem = ui->listFile->count() - 1; // Число элементов в списке
    ui->listFile->setCurrentRow(nItem); // Фокус на последнем элементе (вызов setSignalProperty() по сигналу)
}

// Удалить сигнал
void MainWindow::removeSignal(){
    int deleteInd = ui->listFile->currentRow(); // Индекс элемента для удаления
    if (ui->listFile->count() == 0 || deleteInd == -1) // При отсутствии элементов в списке
        return;
    if (statSignal_.removeSignal(deleteInd) == 0){ // Если удаление прошло успешно
        statusBar()->showMessage("Сигнал " + ui->listFile->item(deleteInd)->text() + " успешно удален"); // Вывод сообщения в statusBar
        ui->listFile->takeItem(deleteInd); // Удаляем элемент из списка
        setStatEstimationBoundaries(statSignal_.getEstimationBoundaries()); // Правка расчетных границ
        setBoundariesShowParams(); // Выставление граничных значений параметров
        removeGraph(deleteInd); // Удаление графика
        plotAllColorMap(); // Построение цветовой карты
    }
}

// Сохранить сигнал
void MainWindow::saveSignalCharacteristics(){
    int saveInd = ui->listFile->currentRow(); // Индекс файла для сохранения
    if (saveInd == -1) return; // Проверка на пустоту списка
    signalCharacteristicsWindow_->setDataSignal(vecDataSignal_[saveInd]); // Передача сигнала
    signalCharacteristicsWindow_->setEstimationBoundaries(statSignal_.getEstimationBoundaries()); // Границы расчета
    signalCharacteristicsWindow_->setLastPath(lastPath_); // Передача пути по умолчанию
    signalCharacteristicsWindow_->show(); // Отображение диалогового окна
}

// Сохранение результатов расчета
void MainWindow::saveCalcualtion(){
    QString saveDir = QFileDialog::getExistingDirectory(this, "", lastPath_, QFileDialog::ShowDirsOnly); // Диалоговое окно
    if (saveDir.isEmpty()) return; // Проверка корректности выбора
    lastPath_ = saveDir + QDir::separator();
    int exitStatus = statSignal_.writeAllStatistics(lastPath_); // Запись статистик
    exitStatus += statSignal_.writeAllMetrics(lastPath_, "Метрики сигналов"); // Запись метрик сигналов
    if (exitStatus == 0)
        statusBar()->showMessage("Результаты сохранены");

}

// Сохранить разбиения по уровням
void MainWindow::saveLevels(){
    if (ui->listFile->count() == 0) return; // Проверка на пустоту списка
    levelsWindow_->show(); // Отображение диалогового окна
}


// -----------------------------------------------------------------------------------------------------------------------
