#include <QMessageBox>
#include <QKeyEvent>
#include <qmath.h>
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
    connect(ui->checkBoxLinearFilter, SIGNAL(stateChanged(int)), this, SLOT(setLinearFilterState())); // Установить состояния линейного фильтра
    connect(ui->spinBoxScanPeriod, SIGNAL(valueChanged(int)), this, SLOT(setScanPeriod(int))); // Установить период опроса
    connect(ui->spinBoxLowerFrequency, SIGNAL(editingFinished()), this, SLOT(checkBandpassFrequencies())); // При изменении нижней частоты
    connect(ui->spinBoxUpperFrequency, SIGNAL(editingFinished()), this, SLOT(checkBandpassFrequencies())); // При изменении верхней частоты
    connect(ui->spinBoxWeightWindowWidth, SIGNAL(editingFinished()), this, SLOT(checkWeightWindowWidth())); // При изменении ширины весового окна
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

// Задание границ данных
void FilterSignalsWindow::setBoundaries(){
    int indOfShortestSignal = minOrMaxByLength(vecDataSignal_, ExtremaOption::MIN); // Находим индекс самого короткого сигнала
    // Задание максимального времени
    ui->spinBoxRightTimeBoundary->setMaximum(vecDataSignal_[indOfShortestSignal].timeDuration()); // По левой границе
    ui->spinBoxLeftTimeBoundary->setMaximum(ui->spinBoxRightTimeBoundary->maximum()); // По правой границе
    ui->spinBoxRightTimeBoundary->setValue(ui->spinBoxRightTimeBoundary->maximum()); // Ставим время усечения по умолчанию равным максимальному
    // Настройка нижней и верхней частот пропускания
    double maxFreq = vecDataSignal_[0].nyquistFrequency(); // Максимально возможная частота в спектре
    ui->spinBoxLowerFrequency->setMaximum(maxFreq); // Нижняя частота
    ui->spinBoxUpperFrequency->setMaximum(maxFreq); // Верхняя частота
}

// Проверка пользовательских частот для фильтрации
void FilterSignalsWindow::checkBandpassFrequencies(){
    double lowerFreq = ui->spinBoxLowerFrequency->value(); // Нижняя частота
    double upperFreq = ui->spinBoxUpperFrequency->value(); // Верхняя частота
    // Проверка превышения нижней частоты значения верхней
    if ( lowerFreq > upperFreq ){
        ui->spinBoxLowerFrequency->setValue(upperFreq);
        ui->spinBoxUpperFrequency->setValue(lowerFreq);
    }
}

// Проверка ширины весового окна
void FilterSignalsWindow::checkWeightWindowWidth(){
    int weightWindowWidth = ui->spinBoxWeightWindowWidth->value(); // Текущая ширина окна
    int weightWindowWidth2 = static_cast<int>(qPow(2, previousPow2(weightWindowWidth))); // Ближайшая ширина окна, кратная двум
    // Установка ширины окна, кратной двум
    if (weightWindowWidth != weightWindowWidth2)
        ui->spinBoxWeightWindowWidth->setValue(weightWindowWidth2);
}

// При отображении виджета
void FilterSignalsWindow::showEvent(QShowEvent * event){
    setBoundaries(); // Задание границ данных
    QWidget::showEvent(event);
}

// При нажатии клавиш
void FilterSignalsWindow::keyPressEvent(QKeyEvent * event)
{
    if ( event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return )
        return;
    QDialog::keyPressEvent(event);
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

// Установить состояния линейного фильтра
void FilterSignalsWindow::setLinearFilterState(){
    bool state = ui->checkBoxLinearFilter->isChecked();
    ui->spinBoxLeftLinearBoundary->setEnabled(state);
    ui->spinBoxRightLinearBoundary->setEnabled(state);
}

// Установить период опроса
void FilterSignalsWindow::setScanPeriod(int newVal){
    ui->spinBoxScanPeriod->blockSignals(true);
    bool isFound = availableScanPeriods_.find(newVal) != availableScanPeriods_.end(); // Флаг наличия выбранного периода опроса в списке доступных
    if ( !isFound ){
        if (newVal > lastScanPeriod_){
            lastScanPeriod_ = *availableScanPeriods_.upper_bound(newVal);
        } else {
            auto it = availableScanPeriods_.lower_bound(newVal);
            if ( it != availableScanPeriods_.cbegin() )
                --it;
            lastScanPeriod_ = *it;
        }
    } else {
        lastScanPeriod_ = newVal;
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
        // Частотный фильтр
    bool isBandpassFilter = ui->groupBoxBandpassFilter->isChecked();
    WindowFunction windowFun = WindowFunction(ui->comboBoxWeightWindowType->currentIndex()); // Тип окна (HAMMING, HANN, BLACKMAN)
    int weightWindowWidth = ui->spinBoxWeightWindowWidth->value(); // Ширина весового окна
    double overlapFactor = ui->spinBoxOverlapFactor->value(); // Коэффициент перекрытия окон
    double lowerFreq = ui->spinBoxLowerFrequency->value(); // Нижняя частота
    double upperFreq = ui->spinBoxUpperFrequency->value(); // Верхняя частота
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
                vecDataSignal_[iSignal] = approximateSmoothSpline(vecDataSignal_[iSignal], 1.0, nPoints);
                break;
            }
        }
        // Срез сигнала по времени
        vecDataSignal_[iSignal] = sliceByTime(vecDataSignal_[iSignal], leftTimeBoundary, rightTimeBoundary);
        // Линейный фильтр
        if (isLinearFilter)
            vecDataSignal_[iSignal] = linearFilter(vecDataSignal_[iSignal], leftLinearBoundary, rightLinearBoundary);
        // Частотный фильтр
        if (isBandpassFilter)
             vecDataSignal_[iSignal] = bandpassFilter(vecDataSignal_[iSignal], windowFun, weightWindowWidth, overlapFactor, {lowerFreq, upperFreq});
        // Назначаем имя файла
        vecDataSignal_[iSignal].setFileName(saveFileName);
    }
    emit accepted(); // Сигнал завершения фильтрации
    hide(); // Скрытие окна
}
