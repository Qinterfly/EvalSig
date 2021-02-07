#include "MainWindow.h"
#include "ui_MainWindow.h"

// ----  Чтение, запись и удаление сигналов ------------------------------------------------------------------------

// Добавить сигнал
void MainWindow::addSignal(int shiftRead){
    // Организация диалога с пользователем
    QStringList listFullFilePath = QFileDialog::getOpenFileNames(this, "Выберите один или несколько файлов с временными сигналами", lastPath_, "Text files (*.txt)");
    if (listFullFilePath.isEmpty()) return;
    int indFile = 0; // Номер файла в обработке
    for ( QString const& fullFilePath : listFullFilePath ){
        int exitStatus = 0; // Код возврата
        ++indFile; // Инкремент номера обрабатываемого файла
        QFileInfo infoName(fullFilePath); // Создание информационного объекта
        QString fileName = infoName.fileName(); // Имя файла
        if (indFile == 1) // Выставление релевантного пути по первому файлу
            lastPath_ = infoName.absolutePath() + QDir::separator(); // Путь к файлу ( + запись в последний выбранный)
        // Добавление сигнала в контейнеры
        DataSignal currentSignal;
        exitStatus += currentSignal.readDataFile(lastPath_, fileName, shiftRead); // Чтение сигнала из файла
         // Расчет статистик + пополнение вектора сигналов
        if (exitStatus == 0)
            exitStatus += statSignal_.addSignal(currentSignal);
        // При успешном добавлении в контейнер
        if (exitStatus == 0){
            QListWidgetItem * item = new QListWidgetItem(infoName.baseName()); // Занесение в список имени без расширения
            // Получение доступно цвета для графика
            QColor colorGraph = Qt::darkGray;
            int iColor = availableColors_.indexOf(true);
            if ( iColor != -1 ){
                colorGraph = colorList_[iColor];
                availableColors_[iColor] = false;
            }
            item->setData(Qt::DecorationRole, colorGraph); // Задание цвета для сигнала
            ui->listFile->insertItem(ui->listFile->count(), item); // Запись элемента в список
            ui->comboBoxRegression->insertItem(ui->listFile->count(), item->text()); // Запись элемента в список регрессии
            setStatEstimationBoundaries(statSignal_.getEstimationBoundaries()); // Правка расчетных границ
            setBoundariesShowParams(); // Выставление границ
            addGraph(); // Добавления сигнала для отрисовки
            statusBar()->showMessage("Сигнал " + fileName + " успешно прочитан"); // Вывод сообщения в statusBar
        }
    }
    ui->comparePlot->replot(); // Обновление окна построения графиков для сравнения
    plotAllColorMap(); // Построение цветовых карт
    // Установка свойств по последнему сигналу
    int nItem = ui->listFile->count() - 1; // Число элементов в списке
    ui->listFile->setCurrentRow(nItem); // Фокус на последнем элементе (вызов setSignalProperty() по сигналу)
}

// Добавить сигнал со смещением
void MainWindow::addShiftSignal(){
    bool resDialog;
    int shift = QInputDialog::getInt(this, "Задание смещения при чтении", "Величина смещения:", 0, -1e5, 1e5, 1, &resDialog); // Диалоговое окно
    if (resDialog)
        addSignal(shift); // Добавление сигналов со смещением
}

// Удалить сигнал
void MainWindow::removeSignal(bool isReplot){
    int deleteInd = ui->listFile->currentRow(); // Индекс элемента для удаления
    if (ui->listFile->count() == 0 || deleteInd == -1) // При отсутствии элементов в списке
        return;
    if (statSignal_.removeSignal(deleteInd) == 0){ // Если удаление прошло успешно
        statusBar()->showMessage("Сигнал " + ui->listFile->item(deleteInd)->text() + " успешно удален"); // Вывод сообщения в statusBar
        int iColor = colorList_.indexOf( ui->listFile->item(deleteInd)->data(Qt::DecorationRole).value<QColor>() );
        if (iColor != -1) availableColors_[iColor] = true; // Делаем цвет вновь доступным для выбора
        ui->listFile->takeItem(deleteInd); // Удаляем элемент из списка
        ui->comboBoxRegression->removeItem(deleteInd); // Удаляем элемент из списка регрессии
        setStatEstimationBoundaries(statSignal_.getEstimationBoundaries()); // Правка расчетных границ
        setBoundariesShowParams(); // Выставление граничных значений параметров
        removeGraph(deleteInd); // Удаление графика
        plotAllColorMap(); // Построение цветовой карты
    }
    if ( isReplot ) ui->comparePlot->replot(); // Обновление окна построения графиков для сравнения
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
    int exitStatus = statSignal_.writeAllStatistics(lastPath_, true); // Запись статистик
    exitStatus += statSignal_.writeAllMetrics(lastPath_, "Метрики сигналов"); // Запись метрик сигналов
    if (exitStatus == 0)
        statusBar()->showMessage("Результаты сохранены");
}

// Сохранить разбиения по уровням
void MainWindow::saveLevels(){
    if (ui->listFile->count() == 0) return; // Проверка на пустоту списка
    levelsWindow_->setSignalsName(*ui->listFile);
    levelsWindow_->setLastPath(lastPath_);
    levelsWindow_->setEstimationBoundaries(statSignal_.getEstimationBoundaries());
    levelsWindow_->show(); // Отображение диалогового окна
}

// Сохранить скриншот программы
void MainWindow::saveScreenshot(){
    static QString const IMAGE_FORMAT = "png";
    // Получение указателя на экран
    QScreen * screen = QGuiApplication::primaryScreen();
    if (const QWindow *window = windowHandle())
        screen = window->screen();
    if (!screen)
        return;
    QPixmap originalPixmap = screen->grabWindow(this->winId()); // Захват текущего окна программы
    // Создание диалога сохранения
    QString fileName = lastPath_ + ui->showModeWidget->tabText(ui->showModeWidget->currentIndex()) + "." + IMAGE_FORMAT;
    QFileDialog fileDialog(this, tr("Сохранить скриншот как"), fileName);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setFileMode(QFileDialog::AnyFile);
    // Настройка типов изображений для отображения
    QStringList mimeTypes;
    const QList<QByteArray> baMimeTypes = QImageWriter::supportedMimeTypes();
    for (const QByteArray &bf : baMimeTypes)
        mimeTypes.append(QLatin1String(bf));
    fileDialog.setMimeTypeFilters(mimeTypes);
    fileDialog.selectMimeTypeFilter("image/" + IMAGE_FORMAT);
    fileDialog.setDefaultSuffix(IMAGE_FORMAT);
    if (fileDialog.exec() != QDialog::Accepted) // Обработка ошибки выбора
        return;
    // Сохранение скриншота
    fileName = fileDialog.selectedFiles().first();
    lastPath_ = fileDialog.directory().path() + QDir::separator(); // Запоминаем директорию для следующей операции
    if (originalPixmap.save(fileName))
        ui->statusBar->showMessage("Сохранение скриншота выполнено успешно");
}

// Сохранить относительные статистики
void MainWindow::saveAssociatedStatistics(){
    if (vecDataSignal_.size() < 2) return; // Проверка числа сигналов для сравнения
    associatedStatisticsWindow_->setLastPath(lastPath_); // Передача последнего пути, выбранного пользователем
    associatedStatisticsWindow_->setSignalsName(*ui->listFile); // Передача имен сигналов
    associatedStatisticsWindow_->setParamsBoundaries(); // Выставление границ параметров окна
    associatedStatisticsWindow_->refreshNumberOfWindows(); // Обновление чисел окон по сигналам
    associatedStatisticsWindow_->show(); // Отобразить окно относительных статистик
}

// Фильтрация сигналов
void MainWindow::filterSignals(){
    if ( vecDataSignal_.isEmpty() ) return;
    filterSignalsWindow_->setLastPath(lastPath_); // Передача последнего пути, выбранного пользователем
    filterSignalsWindow_->setSignalsName(*ui->listFile); // Передача имен сигналов
    filterSignalsWindow_->show(); // Отобразить окно фильтрации сигналов
}

// -----------------------------------------------------------------------------------------------------------------------
