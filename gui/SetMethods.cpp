#include <QDir>
#include <QMouseEvent>
#include "MainWindow.h"
#include "ui_MainWindow.h"

// ---- Методы для определения параметров программы -------------------------------------------------------------------------

// Установка свойств сигнала
void MainWindow::setSignalProperty(){
    int signalInd = ui->listFile->currentRow(); // Индекс сигнала в фокусе
    if (signalInd == -1){ // Проверка на пустоту списка исходных сигналов
        clearSignalPropertyList(); // Очистка листа с свойствами сигнала
        return;
    }
    PropertyDataSignal const& propSignal = vecDataSignal_[signalInd].getProperty();          // Получение свойств по индексу
    ui->tableFileProperty->item(0, 1)->setText(propSignal.fileName_);                        // Имя файла
    ui->tableFileProperty->item(1, 1)->setText(propSignal.dateAndTime_);                     // Дата и время
    ui->tableFileProperty->item(2, 1)->setText(propSignal.measureObject_);                   // Объект измерения
    ui->tableFileProperty->item(3, 1)->setText(propSignal.measurePoint_);                    // Точка установки
    ui->tableFileProperty->item(4, 1)->setText(propSignal.currentCount_);                    // Текущие отсчеты
    ui->tableFileProperty->item(5, 1)->setText(QString::number(propSignal.temperature_));    // Температура
    ui->tableFileProperty->item(6, 1)->setText(propSignal.sensorType_);                      // Тип датчика
    ui->tableFileProperty->item(7, 1)->setText(QString::number(propSignal.physicalFactor_)); // Физический коэффициент
    ui->tableFileProperty->item(8, 1)->setText(propSignal.measureUnit_);                     // Единица измерения
    ui->tableFileProperty->item(9, 1)->setText(QString::number(propSignal.scanPeriod_));     // Период опроса датчика
    ui->tableFileProperty->item(10, 1)->setText(propSignal.characteristic_);                 // Характеристика
    ui->tableFileProperty->item(11, 1)->setText(QString::number(propSignal.nCount_));        // Количество отсчетов
    // Цвет графика
    QListWidgetItem * listItem = ui->listFile->item(signalInd); // Получение элемента списка
    QVariant varItem = listItem->data(Qt::DecorationRole); // Получение цвета в формате QVariant
    ui->tableFileProperty->item(12, 1)->setText(varItem.value<QColor>().name()); // Запись QVariant -> QColor
    ui->tableFileProperty->item(12, 1)->setData(Qt::DecorationRole, listItem->data(Qt::DecorationRole));
}

// Установка цвета сигнала
void MainWindow::setColor(int row, int column){
    int currentSignalInd = ui->listFile->currentRow(); // Индекс выбранного сигнала
    if (currentSignalInd == -1) return; // Проверка на наличие выбранного элемента
    if (row != 12 || column != 1) return; // Проверка выбора ячейки с цветом
    QColor selectedColor = QColorDialog::getColor(Qt::white, this, "Выберите цвет сигнала");
    if (!selectedColor.isValid()) return; // Проверка выбора корректного цвета
    ui->listFile->currentItem()->setData(Qt::DecorationRole, selectedColor); // Установка цвета в списке сигналов
    ui->tableFileProperty->item(row, column)->setData(Qt::DecorationRole, selectedColor); // Установка цвета в свойствах сигнала
    ui->tableFileProperty->item(row, column)->setText(selectedColor.name()); // Имя цвета в hex
    // Для графиков
    ui->comparePlot->graph(SECONDARY_PLOT_IND + currentSignalInd)->setPen(QPen(selectedColor)) ; // Выставление цвета графика в окне "Сравнение"
    ui->comparePlot->replot(); // Обновление построения
}

// Установка параметров окна
void MainWindow::setTimeWindowProperty(){
    widthTimeWindow_ = ui->spinBoxTimeWidth->value(); // Ширина временного окна
    shiftWindow_ = ui->spinBoxShiftWindow->value(); // Смещение временного окна
    if (!statSignal_.isEmpty()){
        ui->spinBoxTimeWidth->setStatusTip("Ширина временного окна = " + QString::number(vecDataSignal_[0].convertCountToTime(widthTimeWindow_)) + " c");
        ui->spinBoxShiftWindow->setStatusTip("Смещение временного окна = " + QString::number(vecDataSignal_[0].convertCountToTime(shiftWindow_)) + " c");
    }
    statSignal_.setWindowProperty(widthTimeWindow_, shiftWindow_); // Передача параметров контейнеру статистик
    setBoundariesShowParams(); // Выставление граничных значений параметров
    updateStatusBar(); // Обновление информационной строки
}

// Установка параметров отображения
void MainWindow::setShowParams(){
    // Проверка необходимости отображение среднего окна
    if (!ui->checkBoxMiddleWindowMode->isChecked()){
        ui->spinBoxShowWindow->setEnabled(true); // Включение возможности выбора номера окна
        showWindow_ = ui->spinBoxShowWindow->value() - 1; // Номер окна для временного окна для отображения
    }
    else {
        ui->spinBoxShowWindow->setEnabled(false); // Отключение возможности выбора номера окна
        showWindow_ = ui->spinBoxShowWindow->maximum(); // Отображение среднего окна
    }
    // Проверка необходимости интерполяции
    bool isInterpolate = ui->checkBoxInterpolateColorMap->isChecked();
    int nCMapPlot = vecColorMap_.size(); // Число цветовых карт
    for (int plotInd = 0; plotInd != nCMapPlot; ++plotInd){
        vecColorMap_[plotInd]->setInterpolate(isInterpolate); // Выставить состояние интерполяции
    }
    plotAllColorMap(); // Построение цветовой карты
}

// Выставления границ отображения для объектов интерфейса
void MainWindow::setBoundariesShowParams(){
    int nWindows = statSignal_.getNumberOfWindows(); // Число временных окон
    // Проверка пустоты статистик и необходимости смены предела
    if (!statSignal_.isEmpty() && nWindows != ui->spinBoxShowWindow->maximum()){
        ui->spinBoxShowWindow->setMaximum(nWindows); // Номер отображемого окна
        setShowParams(); // Выставление параметров
    }
}

// Изменить отображение списка сигналов
void MainWindow::setVisibleFileWidget(bool isChecked){ ui->dockFileWidget->setVisible(isChecked); }

// Изменить отображение виджета свойств
void MainWindow::setVisiblePropertyWidget(bool isChecked){ ui->dockPropertyWidget->setVisible(isChecked); }

// Установка границ расчета
    // Поля -> статистики
void MainWindow::setStatEstimationBoundaries(){
    // Получение новых границ расчета
    int newLeftBound = ui->spinBoxLeftEstimationBoundary->value(), newRightBound = ui->spinBoxRightEstimationBoundary->value();
    // Задание максимальной правой границы
    if ( newRightBound < 1 ){
        newRightBound = !statSignal_.isEmpty() ? statSignal_.maxSizeSignals() : ui->spinBoxRightEstimationBoundary->maximum();
        ui->spinBoxRightEstimationBoundary->setValue(newRightBound);
    }
    // Проверка допустимости новых значений
    if (newLeftBound >= newRightBound){ // Левая граница превышает или равна правой
        std::swap(newLeftBound, newRightBound);
        ui->spinBoxLeftEstimationBoundary->setValue(newLeftBound);
        ui->spinBoxRightEstimationBoundary->setValue(newRightBound);
    }
    // Проверка выхода расчетной границы за минимальную длину сигнала из группы
    if (!statSignal_.isEmpty() && newRightBound > statSignal_.minSizeSignals()){
        newRightBound = statSignal_.minSizeSignals();
        ui->spinBoxRightEstimationBoundary->setValue(newRightBound);
    }
    statSignal_.setEstimationBoundaries(newLeftBound, newRightBound); // Выставление границ расчета статистик
    setBoundariesShowParams(); // Выставление граничных значений параметров
    plotEstimationBoundaries(true); // Построение граничных линий
    updateStatusBar(); // Обновление информационной строки
    // Отображение временных границ в подсказках
    ui->spinBoxLeftEstimationBoundary->setToolTip(QString::number(vecDataSignal_[0].convertCountToTime(newLeftBound)) + " c");
    ui->spinBoxRightEstimationBoundary->setToolTip(QString::number(vecDataSignal_[0].convertCountToTime(newRightBound)) + " c");
}

    // Статистики -> поля
void MainWindow::setStatEstimationBoundaries(QPair<int, int> const& estimationBoundaries){
    // Получение старых границ расчета
    int oldLeftBound = ui->spinBoxLeftEstimationBoundary->value(), oldRightBound = ui->spinBoxRightEstimationBoundary->value();
    // Получение новых границ расчета
    int newLeftBound = estimationBoundaries.first, newRightBound = estimationBoundaries.second;
    // Проверка необходимости изменений
    if (oldLeftBound == newLeftBound && oldRightBound == newRightBound)
        return;
    // Установка границ
    ui->spinBoxLeftEstimationBoundary->setValue(newLeftBound);
    ui->spinBoxRightEstimationBoundary->setValue(newRightBound);
    setBoundariesShowParams(); // Выставление граничных значений параметров
    plotEstimationBoundaries(true); // Построение граничных линий
    updateStatusBar(); // Обновление информационной строки
    // Отображение временных границ в подсказках
    ui->spinBoxLeftEstimationBoundary->setToolTip(QString::number(vecDataSignal_[0].convertCountToTime(newLeftBound)) + " c");
    ui->spinBoxRightEstimationBoundary->setToolTip(QString::number(vecDataSignal_[0].convertCountToTime(newRightBound)) + " c");
}

// Изменение параметров сигнала
void MainWindow::changeDataSignal(int row, int column){
    if ( column != 1 ) return; // Если не выбрано значение
    int currentSignalInd = ui->listFile->currentRow(); // Индекс выбранного сигнала
    QTableWidgetItem * item = ui->tableFileProperty->item(row, column); // Получение объекта таблицы
    if ( !item->isSelected() ) return; // Если поле не выбрано пользователем
    switch ( row ){
    // Имя файла
    case 0:
    {
        QString const& fileName = item->text(); // Полное имя файла
        vecDataSignal_[currentSignalInd].setFileName(fileName);
        ui->listFile->item(currentSignalInd)->setText(QFileInfo(fileName).baseName()); // Имя в списке без расширения
        break;
    }
    // Время и дата
    case 1:
        vecDataSignal_[currentSignalInd].setDateAndTime(item->text());
        break;
    // Объект измерения
    case 2:
        vecDataSignal_[currentSignalInd].setMeasureObject(item->text());
        break;
    // Точка установки датчика
    case 3:
        vecDataSignal_[currentSignalInd].setMeasurePoint(item->text());
        break;
    // Температура
    case 5:
        vecDataSignal_[currentSignalInd].setTemperature(item->text().toDouble());
        break;
    // Тип датчика
    case 6:
        vecDataSignal_[currentSignalInd].setSensorType(item->text());
        break;
    // Физический коэффициент
    case 7:
    {
        double tVal = item->text().toDouble();
        if ( qAbs(tVal) == 0.0 ) return; // Запрет нулевого коэффициента
        vecDataSignal_[currentSignalInd].setPhysicalFactor(tVal); // Установка коэффициента
        statSignal_.recalculate();      // Пересчет статистик
        replotGraph(currentSignalInd);  // Обновление графика
        plotAllColorMap();              // Построение цветовых карт
        break;
    }
    // Единица измерения
    case 8:
        vecDataSignal_[currentSignalInd].setMeasureUnit(item->text());
        break;
    // Период опроса
    case 9:
    {
        int tVal = item->text().toInt();
        if ( qAbs(tVal) == 0.0 ) return; // Запрет нулевого периода опроса
        vecDataSignal_[currentSignalInd].setScanPeriod(tVal);
        replotGraph(currentSignalInd);  // Обновление графика
        // Отображение временных границ в подсказках
        ui->spinBoxLeftEstimationBoundary->setToolTip(QString::number(vecDataSignal_[0].convertCountToTime(ui->spinBoxLeftEstimationBoundary->value())) + " c");
        ui->spinBoxRightEstimationBoundary->setToolTip(QString::number(vecDataSignal_[0].convertCountToTime(ui->spinBoxLeftEstimationBoundary->value())) + " c");
        break;
    }
    // Характеристика
    case 10:
        vecDataSignal_[currentSignalInd].setCharacteristic(item->text());
        break;
    }
}

// Переопределение событий программы
bool MainWindow::eventFilter(QObject * obj, QEvent * event){
    // Параметры размеров
    static const float RELATIVE_WIDTH_DOCK = 0.3f;
    static const float RELATIVE_WIDTH_MAINWINDOW = 0.4688f;
    static const float RELATIVE_HEIGHT_MAINWINDOW = 0.5556f;
    // В случае изменения размера
    if (event->type() == QEvent::Resize){
        QResizeEvent * resizeEvent = dynamic_cast<QResizeEvent*>(event); // Событие
        // Для главного окна
        if (obj == this){
            // При инициализации окна
            if (resizeEvent->oldSize().width() == -1){
                QRect screenGeometry = QApplication::primaryScreen()->geometry(); // Параметры текущего монитора
                QRect winGeometry = this->geometry(); // Геометрия старого окна
                // Установка параметров окна с учетом разрешения монитора
                    // Размеры
                winGeometry.setWidth(qRound(screenGeometry.width() * RELATIVE_WIDTH_MAINWINDOW));
                winGeometry.setHeight(qRound(screenGeometry.height() * RELATIVE_HEIGHT_MAINWINDOW));

                this->setGeometry(winGeometry); // Установка геометрии
                this->move(qRound((screenGeometry.width() - winGeometry.width()) / 2.0), // Позиция по центру
                           qRound((screenGeometry.height() - winGeometry.height()) / 2.0));
            }
            // Установка процентного максимума от ширины окна для левой панели
            int leftPanelMaxWidth = qRound(this->width() * RELATIVE_WIDTH_DOCK);
            ui->dockFileWidget->setMaximumWidth(leftPanelMaxWidth); // Список сигналов
            ui->dockPropertyWidget->setMaximumWidth(leftPanelMaxWidth); // Свойства
        }
        // Для dock виджетов
        if (obj == ui->dockFileWidget || obj == ui->dockPropertyWidget){
            // Установка правой границы таблицы
            int shiftSize = resizeEvent->size().width() - resizeEvent->oldSize().width(); // Величина смещения
            if (resizeEvent->oldSize().width() == -1) shiftSize = 0; // При инициализации размеров окна
            ui->tableFileProperty->setColumnWidth(1, ui->tableFileProperty->columnWidth(1) + shiftSize); // Смещение границы
        }
    }
    return QObject::eventFilter(obj, event); // Стандартная обработка событий
}

// Применение расчетного шаблона
void MainWindow::applyCalculationTemplate(QVector<int> iSelectedSignals, int iMainSignal, int iBaseSignal, int iSupportSignal){
    int exitStatus = 0;
    // Применение параметров расчета статистик
    ui->spinBoxTimeWidth->setValue(calcTemplate_.widthWindow()); // Ширина окна
    ui->spinBoxShiftWindow->setValue(calcTemplate_.shiftWindow()); // Смещение окна
    QPair<int, int> estimationBoundaries = calcTemplate_.estimationBoundaries();
    ui->spinBoxLeftEstimationBoundary->setValue(estimationBoundaries.first); // Левая граница расчета
    ui->spinBoxRightEstimationBoundary->setValue(estimationBoundaries.second); // Правая граница расчета
    setTimeWindowProperty(); // Установка параметров окна
    setStatEstimationBoundaries(); // Установка границ расчета
    estimationBoundaries = {ui->spinBoxLeftEstimationBoundary->value(), ui->spinBoxRightEstimationBoundary->value()}; // Исправленные границы
    // Применение шаблона по окнам
        // Характеристики сигнала
    signalCharacteristicsWindow_->setEstimationBoundaries(estimationBoundaries); // Установка расчетных границ
    signalCharacteristicsWindow_->applyCalculationTemplate();
        // Относительные статистики
    associatedStatisticsWindow_->setSignalsName(*ui->listFile);
    associatedStatisticsWindow_->applyCalculationTemplate(iMainSignal);
        // Разбиение на уровни
    levelsWindow_->setSignalsName(*ui->listFile);
    levelsWindow_->setEstimationBoundaries(estimationBoundaries); // Установка расчетных границ
    levelsWindow_->applyCalculationTemplate(iBaseSignal, iSupportSignal);
    // Оценка полного числа шагов расчета
    int nSelectedSignals = iSelectedSignals.size(); // Число сигналов
    QList<QString> sequenceOfWindows = calcTemplate_.getSequenceOfWindows(); // Последовательность окон
    int lengthSequence = calcTemplate_.lengthSequence(); // Длина последовательности
    int nStepCalc = 0;
    nStepCalc += sequenceOfWindows.contains("SignalCharacteristicsWindow") ? nSelectedSignals : 0; // Характеристики сигнала
    nStepCalc += sequenceOfWindows.contains("StatisticsWindow") ? 1 : 0; // Статистические коэффициенты
    nStepCalc += sequenceOfWindows.contains("AssociatedStatisticsWindow") ? 1 : 0; // Относительные статистики
    nStepCalc += sequenceOfWindows.contains("LevelsWindow") ? 1 : 0; // Разбиение по уровням
    // Настройка окна с прогрессом
    QProgressDialog progress = QProgressDialog("", "Отменить", 0, nStepCalc, this); // Создание экземпляра окна с прогрессом
    progress.setWindowModality(Qt::WindowModal);
    // Применение шаблона к сигналам по последовательности
    int iStepCalc = 0;
    for (int iSeq = 0; iSeq != lengthSequence; ++iSeq){
        QString const& seq = sequenceOfWindows[iSeq];
        QDir baseDir(lastPath_);
        // Характеристики сигнала
        if ( !seq.compare("SignalCharacteristicsWindow" ) ){
            for (int jSig = 0; jSig != nSelectedSignals; ++jSig){
                int indSignal = iSelectedSignals[jSig]; // Индекс сигнала в контейнере
                QString signalName = ui->listFile->item(indSignal)->text(); // Имя сигнала
                baseDir.mkdir(signalName); // Создаем директорию по имени сигнала
                QString fullSignalPath = lastPath_ + signalName + QDir::separator(); // Директория сигнала
                // Обработка прогресса
                if (progress.wasCanceled()) break;
                progress.setLabelText("Характеристики сигнала: " + signalName + "...");
                progress.setValue(iStepCalc++);
                qApp->processEvents();
                // Вызов расчетных методов
                signalCharacteristicsWindow_->setDataSignal(vecDataSignal_[indSignal]); // Передача сигнала
                signalCharacteristicsWindow_->setLastPath(fullSignalPath); // Передача пути по умолчанию
                signalCharacteristicsWindow_->saveCharacteristics(false); // Сохранение результатов
            }
        }
        // Статистические коэффициенты
        if ( !seq.compare("StatisticsWindow") ){
            if (progress.wasCanceled()) break;
            QString path = lastPath_ + "Статистики" + QDir::separator();
            progress.setLabelText("Статистические коэффициенты...");
            progress.setValue(iStepCalc++);
            qApp->processEvents();
            exitStatus += statSignal_.writeAllStatistics(path); // Запись статистик
            exitStatus += statSignal_.writeAllMetrics(path, "Метрики сигналов"); // Запись метрик сигналов
        }
        // Относительные статистики
        if ( !seq.compare("AssociatedStatisticsWindow") ){
            if (progress.wasCanceled()) break;
            QString mainSignalName = ui->listFile->item(iMainSignal)->text();
            QString path = lastPath_ + "Относительные статистики " + mainSignalName + QDir::separator();
            progress.setLabelText("Относительные статистики " + mainSignalName + "...");
            progress.setValue(iStepCalc++);
            qApp->processEvents();
            associatedStatisticsWindow_->setLastPath(path);
            associatedStatisticsWindow_->save(false);
        }
        // Разбиение на уровни
        if ( !seq.compare("LevelsWindow") ){
            if (progress.wasCanceled()) break;
            QString baseSignalName = ui->listFile->item(iBaseSignal - 1)->text();
            QString path = lastPath_ + "Уровни базового " + baseSignalName + QDir::separator();
            progress.setLabelText("Разбиение на уровни базового: " + baseSignalName + "...");
            progress.setValue(iStepCalc++);
            qApp->processEvents();
            levelsWindow_->setLastPath(path);
            levelsWindow_->save(false);
        }
    }
    progress.setValue(nStepCalc);
    calculationTemplateProcessed(CalculationTemplateWindow::Code::FinishedApplying);
    calcTemplateWindow_->hide();
}

// ----------------------------------------------------------------------------------------------------------------------
