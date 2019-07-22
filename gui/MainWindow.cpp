#include "MainWindow.h"
#include "ui_MainWindow.h"

// Конструктор главного окна
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    statSignal_(vecDataSignal_, widthTimeWindow_, shiftWindow_, 1, 2) // Статистики
{
    ui->setupUi(this); // Инициализация графического интерфейса
    signalCharacteristicsWindow_ = new SignalCharacteristicsWindow(this);
    initializeCalculationParams(); // Выставление расчетных параметров
    clearSignalPropertyList(); // Очистка листа со свойствами сигнала
    initializeSignalPropertyList(); // Инициализация листа со свойствами сигнала
    initializeAllPlot(); // Инициализация графических окон
    initializeShowParams(); // Инициализация параметров для отображения

    // Создание соединений сигнал - слот
    connect(ui->actionAddSignal, SIGNAL(triggered()), this, SLOT(addSignal())); // Добавить сигнал
    connect(ui->actionRemoveSignal, SIGNAL(triggered()), this, SLOT(removeSignal())); // Удалить сигнал
    connect(ui->actionSaveSignal, SIGNAL(triggered()), this, SLOT(saveSignalCharacteristics())); // Сохранить сигнал
    connect(ui->actionSaveCalculation, SIGNAL(triggered()), this, SLOT(saveCalcualtion())); // Сохранить результаты расчета
    connect(ui->listFile, SIGNAL(itemSelectionChanged()), this, SLOT(setSignalProperty()), Qt::QueuedConnection); // Установка свойств сигнала
    connect(ui->actionNewProject, SIGNAL(triggered()), this, SLOT(clearProject())); // Новый проект
    connect(ui->tableFileProperty, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(setColor(int, int))); // Установка цвета сигнала
    connect(ui->tableFileProperty, SIGNAL(cellChanged(int, int)), this, SLOT(changeDataSignal(int, int))); // Установка параметров сигнала
    connect(ui->spinBoxTimeWidth, SIGNAL(editingFinished()), this, SLOT(setTimeWindowProperty())); // Установка параметров окна по ширине
    connect(ui->spinBoxShiftWindow, SIGNAL(editingFinished()), this, SLOT(setTimeWindowProperty())); // Установка параметров окна по коэффициенту перекрытия
    connect(ui->spinBoxShowWindow, SIGNAL(valueChanged(int)), this, SLOT(setShowParams())); // Установка параметров отображения
    connect(ui->actionVisibleFileWidget, SIGNAL(triggered(bool)), this, SLOT(setVisibleFileWidget(bool))); // Изменить отображения списка сигналов
    connect(ui->actionVisiblePropertyWidget, SIGNAL(triggered(bool)), this, SLOT(setVisiblePropertyWidget(bool))); // Изменить отображения свойств проекта
    connect(ui->checkBoxMiddleWindowMode, SIGNAL(clicked(bool)), this, SLOT(setShowParams())); // Переключение режима отображения окон
    connect(ui->checkBoxInterpolateColorMap, SIGNAL(clicked(bool)), this, SLOT(setShowParams())); // Переключение режима интерполяции цветовых карт
    connect(ui->spinBoxLeftEstimationBoundary, SIGNAL(editingFinished()), this, SLOT(setStatEstimationBoundaries())); // Установка границ расчета
    connect(ui->spinBoxRightEstimationBoundary, SIGNAL(editingFinished()), this, SLOT(setStatEstimationBoundaries())); // Установка границ расчета
    // Обновление statusBar
    connect(ui->showModeWidget, SIGNAL(currentChanged(int)), this, SLOT(updateStatusBar())); // При переключении типа графиков
    connect(ui->listFile, SIGNAL(itemSelectionChanged()), this, SLOT(updateStatusBar()), Qt::QueuedConnection); // При выборе сигнала
    // Дополнительные окна
    connect(signalCharacteristicsWindow_, SIGNAL(accepted()), this, SLOT(saveSignalCharacteristicsFinished()));
}

// Деструктор главного окна
MainWindow::~MainWindow()
{
    delete ui;
}
