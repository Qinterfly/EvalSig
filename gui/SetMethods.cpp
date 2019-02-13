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
    PropertyDataSignal const& propSignal = vecDataSignal_[signalInd].getProperty(); // Получение свойств по индексу
    ui->tableFileProperty->item(0, 1)->setText(propSignal.path_ + propSignal.fileName_); // Полный путь к файлу
    ui->tableFileProperty->item(1, 1)->setText(propSignal.dateAndTime_); // Дата и время
    ui->tableFileProperty->item(2, 1)->setText(propSignal.measureObject_); // Объект измерения
    ui->tableFileProperty->item(3, 1)->setText(propSignal.measurePoint_); // Точка установки
    ui->tableFileProperty->item(4, 1)->setText(propSignal.currentCount_); // Текущие отсчеты
    ui->tableFileProperty->item(5, 1)->setText(QString::number(propSignal.temperature_)); // Температура
    ui->tableFileProperty->item(6, 1)->setText(propSignal.sensorType_); // Тип датчика
    ui->tableFileProperty->item(7, 1)->setText(QString::number(propSignal.physicalFactor_)); // Физический коэффициент
    ui->tableFileProperty->item(8, 1)->setText(propSignal.measureUnit_); // Единица измерения
    ui->tableFileProperty->item(9, 1)->setText(QString::number(propSignal.scanPeriod_)); // Период опроса датчика
    ui->tableFileProperty->item(10, 1)->setText(propSignal.characterisic_); // Характеристика
    ui->tableFileProperty->item(11, 1)->setText(QString::number(propSignal.nCount_)); // Количество отсчетов
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
    ui->comparePlot->graph(currentSignalInd)->setPen(QPen(selectedColor)) ; // Выставление цвета графика в окне "Сравнение"
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
    setBoundaryShowParams(); // Выставление граничных значений параметров
    updateStatusBar(); // Обновление информационной строки
}

// Установка параметров отображения
void MainWindow::setShowParams(){
    if (!ui->checkBoxMiddleWindowMode->isChecked()){ // Проверка необходимости отображение среднего окна
        ui->spinBoxShowWindow->setEnabled(true); // Включение возможности выбора номера окна
        showWindow_ = ui->spinBoxShowWindow->value() - 1; // Номер окна для временного окна для отображения
    }
    else{
        ui->spinBoxShowWindow->setEnabled(false); // Отключение возможности выбора номера окна
        showWindow_ = ui->spinBoxShowWindow->maximum(); // Отображение среднего окна
    }
    plotAllColorMap(); // Построение цветовой карты
}

// Выставления границ отображения для объектов интерфейса
void MainWindow::setBoundaryShowParams(){
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

// Переопределение событий программы
bool MainWindow::eventFilter(QObject * obj, QEvent * event){
    // Параметры размеров
    static const float RELATIVE_WIDTH_DOCK = 0.3f;
    static const float RELATIVE_WIDTH_MAINWINDOW = 0.4688f;
    static const float RELATIVE_HEIGHT_MAINWINDOW = 0.5556f;
    // В случае изменения размера
    if (event->type() == QEvent::Resize){
        QResizeEvent * resizeEvent = static_cast<QResizeEvent*>(event); // Событие
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

// ----------------------------------------------------------------------------------------------------------------------
