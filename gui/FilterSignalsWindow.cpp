#include "FilterSignalsWindow.h"
#include "ui_FilterSignalsWindow.h"

// ---- Фильтрация сигналов ------------------------------------------------------------------------------------

// Конструктор

FilterSignalsWindow::FilterSignalsWindow(QVector<DataSignal> const& vecDataSignal, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FilterSignalsWindow),
    vecDataSignal_(vecDataSignal)
{
    ui->setupUi(this);
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
}

// Установка пути по умолчанию
void FilterSignalsWindow::setLastPath(QString const& lastPath){
    lastPath_ = lastPath;
}

// Задание временных границ
void FilterSignalsWindow::setTimeBoundaries(){
    int indOfShortestSignal = minOrMaxByLength(vecDataSignal_, ExtremaOption::MIN); // Находим индекс самого короткого сигнала
    // Задаем максимальное время
    ui->spinBoxRightTimeBoundary->setMaximum(vecDataSignal_[indOfShortestSignal].timeDuration()); // По левой границе
    ui->spinBoxLeftTimeBoundary->setMaximum(ui->spinBoxRightTimeBoundary->maximum()); // По правой границе
}

// При отображении виджета
void FilterSignalsWindow::showEvent(QShowEvent * event){
    setTimeBoundaries(); // Задание временных границ
    QWidget::showEvent(event);
}
