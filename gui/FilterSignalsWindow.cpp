#include <QMessageBox>
#include "FilterSignalsWindow.h"
#include "ui_FilterSignalsWindow.h"
#include "core/NumericalFunctions.h"

// ---- Фильтрация сигналов ------------------------------------------------------------------------------------

// Конструктор

FilterSignalsWindow::FilterSignalsWindow(QVector<DataSignal> & vecDataSignal, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FilterSignalsWindow),
    vecDataSignal_(vecDataSignal)
{
    ui->setupUi(this);
    // Создание соединений сигнал - слот
    connect(ui->listSignals, SIGNAL(itemSelectionChanged()), this, SLOT(checkStateFilter())); // Проверка возможности расчета
    connect(ui->spinBoxLeftTimeBoundary, SIGNAL(editingFinished()), this, SLOT(checkTimeBoundaries())); // Проверка левой временной границы
    connect(ui->spinBoxRightTimeBoundary, SIGNAL(editingFinished()), this, SLOT(checkTimeBoundaries())); // Проверка правой временной границы
    connect(ui->buttonFilter, SIGNAL(clicked()), this, SLOT(filterSignals())); // Фильтрация сигналов
    connect(ui->checkBoxOutlier, SIGNAL(stateChanged(int)), this, SLOT(setOutlierState())); // Установка состояние выбора предельного значения выбросов
    connect(ui->spinBoxScanPeriod, SIGNAL(valueChanged(int)), this, SLOT(setScanPeriod(int))); // Установить период опроса
}

// Деструктор
FilterSignalsWindow::~FilterSignalsWindow(){
    delete ui;
}

// Определение имен сигналов для выбора
void FilterSignalsWindow::setSignalsName(QListWidget const& listWidgetSignals){
    int nItem = listWidgetSignals.count();
    ui->listSignals->clear(); // Очистка списка сигналов
    // Добавление имен сигналов в список
    for (int i = 0; i != nItem; ++i){
        QString const& tName = listWidgetSignals.item(i)->text();
        ui->listSignals->addItem(tName);
    }
    ui->listSignals->selectAll();
}

// Установка пути по умолчанию
void FilterSignalsWindow::setLastPath(QString const& lastPath){
    lastPath_ = lastPath;
}

// Задание временных границ
void FilterSignalsWindow::setTimeLimits(){
    int indOfShortestSignal = minOrMaxByLength(vecDataSignal_, ExtremaOption::MIN); // Находим индекс самого короткого сигнала
    // Задание максимального времени
    ui->spinBoxRightTimeBoundary->setMaximum(vecDataSignal_[indOfShortestSignal].timeDuration()); // По левой границе
    ui->spinBoxLeftTimeBoundary->setMaximum(ui->spinBoxRightTimeBoundary->maximum()); // По правой границе
    ui->spinBoxRightTimeBoundary->setValue(ui->spinBoxRightTimeBoundary->maximum()); // Ставим время усечения по умолчанию равным максимальному
}

// При отображении виджета
void FilterSignalsWindow::showEvent(QShowEvent * event){
    setTimeLimits(); // Задание временных границ
    QWidget::showEvent(event);
}

// Проверка возможности расчета
void FilterSignalsWindow::checkStateFilter(){
    const int limDivisors = ui->spinBoxScanPeriod->maximum();
    if ( !ui->listSignals->selectedItems().isEmpty() ){
        ui->buttonFilter->setEnabled(true);
        // Задание значений для выбора периода опроса
        ui->spinBoxScanPeriod->setEnabled(true);
        QModelIndexList const& selectedIndexes = ui->listSignals->selectionModel()->selectedIndexes();
        int iSelected = selectedIndexes[0].row();
        availableScanPeriods_ = findDivisors(vecDataSignal_[iSelected].timeDuration() / DataSignal::TIME_PHYS_MULT, limDivisors); // Получаем все делители до верхней границы
        // Задание период опроса по умолчанию
        ui->spinBoxScanPeriod->blockSignals(true);
        lastScanPeriod_ = vecDataSignal_[iSelected].getProperty().scanPeriod_;
        ui->spinBoxScanPeriod->setValue(lastScanPeriod_);
        ui->spinBoxScanPeriod->setMaximum(*--availableScanPeriods_.end()); // Изменяем максимальный период опроса
        ui->spinBoxScanPeriod->blockSignals(false);
        // Задание максимального числа точек для линейного фильтра
        ui->spinBoxLeftLinearBoundary->setMaximum(vecDataSignal_[iSelected].size() / 2);
        ui->spinBoxRightLinearBoundary->setMaximum(ui->spinBoxLeftLinearBoundary->maximum());
    } else {
        ui->buttonFilter->setEnabled(false);
        ui->spinBoxScanPeriod->setEnabled(false);
    }
}

// Проверка временных границ
void FilterSignalsWindow::checkTimeBoundaries(){
    double lBound = ui->spinBoxLeftTimeBoundary->value();
    double rBound = ui->spinBoxRightTimeBoundary->value();
    if( lBound > rBound ){
        std::swap(lBound, rBound);
        ui->spinBoxLeftTimeBoundary->setValue(lBound);
        ui->spinBoxRightTimeBoundary->setValue(rBound);
    }
}

// Установка состояние выбора предельного значения выбросов
void FilterSignalsWindow::setOutlierState(){
    ui->spinBoxOutlier->setEnabled(ui->checkBoxOutlier->isChecked());
}

// Установить период опроса
void FilterSignalsWindow::setScanPeriod(int newVal){
    ui->spinBoxScanPeriod->blockSignals(true);
    if (newVal > lastScanPeriod_){
        lastScanPeriod_ = *availableScanPeriods_.upper_bound(newVal);
    } else {
        auto it = availableScanPeriods_.lower_bound(newVal);
        if (it != availableScanPeriods_.cbegin()) --it;
        lastScanPeriod_ = *it;
    }
    ui->spinBoxScanPeriod->setValue(lastScanPeriod_);
    ui->spinBoxScanPeriod->blockSignals(false);
}

// Фильтрация сигналов
void FilterSignalsWindow::filterSignals(){
    QModelIndexList const& selectedIndexes = ui->listSignals->selectionModel()->selectedIndexes();
    int nSelected = selectedIndexes.size(); // Число выбранных сигналов
    int k = vecDataSignal_[selectedIndexes[0].row()].size();
    for (int i = 1; i != nSelected; ++i){
        if (vecDataSignal_[selectedIndexes[i].row()].size() != k){
            QMessageBox(QMessageBox::Warning, "Ошибка выбора сигналов", "Выбранные для фильтрации сигналы имеют разную длину").exec();
            return;
        }
    }
    // Параметры фильтрации
        // Временные границы
    double leftTimeBoundary = ui->spinBoxLeftTimeBoundary->value();
    double rightTimeBoundary = ui->spinBoxRightTimeBoundary->value();
        // Исключение выбросов
    bool isExcludeOutliers = ui->checkBoxOutlier->isChecked();
    double limOutlier = ui->spinBoxOutlier->value();
        // Линейный фильтр
    bool isLinearFilter = ui->checkBoxLinearFilter->isChecked();
    int leftLinearBoundary = ui->spinBoxLeftLinearBoundary->value();
    int rightLinearBoundary = ui->spinBoxRightLinearBoundary->value();
        // Интерполяция
    bool isInterpolate = ui->groupBoxInterpolation->isChecked();
    int nPoints = vecDataSignal_[0].timeDuration() / DataSignal::TIME_PHYS_MULT / ui->spinBoxScanPeriod->value(); // Всегда целое
    // Последовательная обработка выбранных сигналов
    QString saveFileName; // Сохранненое имя файла
    int iSignal = -1; // Индекс выбранного сигнала
    for (int i = 0; i != nSelected; ++i){
        iSignal = selectedIndexes[i].row();
        saveFileName = vecDataSignal_[iSignal].getName();
        // Исключение выбросов
        if (isExcludeOutliers)
            vecDataSignal_[iSignal] = excludeOutliers(vecDataSignal_[iSignal], limOutlier);
        // Интерполяция
        if ( isInterpolate ){
            switch ( ui->comboBoxInterpolationType->currentIndex() ){
            case InterpolationType::LINEAR:
                vecDataSignal_[iSignal] = interpolateLinear(vecDataSignal_[iSignal], nPoints, false);
                break;
            case InterpolationType::SPLINE:
                vecDataSignal_[iSignal] = interpolateSpline(vecDataSignal_[iSignal], {0, vecDataSignal_[iSignal].timeDuration()} , nPoints, false);
                break;
            }
        }
        // Срез сигнала по времени
        vecDataSignal_[iSignal] = sliceByTime(vecDataSignal_[iSignal], leftTimeBoundary, rightTimeBoundary);
        // Линейный фильтр
        if (isLinearFilter)
            vecDataSignal_[iSignal] = linearFilter(vecDataSignal_[iSignal], leftLinearBoundary, rightLinearBoundary);
        // Назначаем имя файла
        vecDataSignal_[iSignal].setFileName(saveFileName);
    }
    emit accepted(); // Сигнал завершения фильтрации
    hide(); // Скрытие окна
}
