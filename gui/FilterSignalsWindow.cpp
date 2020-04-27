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
    connect(ui->listSignals, SIGNAL(itemSelectionChanged()), this, SLOT(checkStatePerform()), Qt::QueuedConnection); // Проверка возможности расчета
    connect(ui->spinBoxLeftTimeBoundary, SIGNAL(editingFinished()), this, SLOT(checkTimeBoundaries())); // Проверка левой временной границы
    connect(ui->spinBoxRightTimeBoundary, SIGNAL(editingFinished()), this, SLOT(checkTimeBoundaries())); // Проверка правой временной границы
    connect(ui->buttonFilter, SIGNAL(clicked()), this, SLOT(filterSignals())); // Фильтрация сигналов
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
    ui->listSignals->setCurrentRow(0); // Выбор первого сигнала по умолчанию
    ui->listSignals->selectAll();
}

// Установка пути по умолчанию
void FilterSignalsWindow::setLastPath(QString const& lastPath){
    lastPath_ = lastPath;
}

// Задание временных границ
void FilterSignalsWindow::setTimeLimits(){
    int indOfShortestSignal = minOrMaxByLength(vecDataSignal_, ExtremaOption::MIN); // Находим индекс самого короткого сигнала
    // Задаем максимальное время
    ui->spinBoxRightTimeBoundary->setMaximum(vecDataSignal_[indOfShortestSignal].timeDuration()); // По левой границе
    ui->spinBoxLeftTimeBoundary->setMaximum(ui->spinBoxRightTimeBoundary->maximum()); // По правой границе
}

// При отображении виджета
void FilterSignalsWindow::showEvent(QShowEvent * event){
    setTimeLimits(); // Задание временных границ
    QWidget::showEvent(event);
}

// Проверка возможности расчета
void FilterSignalsWindow::checkStatePerform(){
    if ( !ui->listSignals->selectedItems().isEmpty() )
        ui->buttonFilter->setEnabled(true);
    else
        ui->buttonFilter->setEnabled(false);
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

// Фильтрация сигналов
void FilterSignalsWindow::filterSignals(){
    // Параметры фильтрации
        // Временные границы
    double leftTimeBoundary = ui->spinBoxLeftTimeBoundary->value();
    double rightTimeBoundary = ui->spinBoxRightTimeBoundary->value();
    // Последовательная обработка выбранных сигналов
    QModelIndexList const& selectedIndexes = ui->listSignals->selectionModel()->selectedIndexes();
    int nSelected = selectedIndexes.size(); // Число выбранных сигналов
    int iSignal = -1; // Индекс выбранного сигнала
    for (int i = 0; i != nSelected; ++i){
        iSignal = selectedIndexes[i].row();
        // Срез сигнала по времени
        vecDataSignal_[iSignal] = sliceByTime(vecDataSignal_[iSignal], leftTimeBoundary, rightTimeBoundary);
    }
    accepted(); // Сигнал завершения фильтрации
    hide(); // Скрытие окна
}
