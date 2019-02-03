#include "MainWindow.h"
#include "ui_MainWindow.h"

// Конструктор главного окна
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    statSignal_(vecDataSignal_, widthTimeWindow_, overlapFactor_) // Статистики
{
    ui->setupUi(this); // Инициализация графического интерфейса
    initializeCalculationParams(); // Выставление расчетных параметров
    clearSignalPropertyList(); // Очистка листа со свойствами сигнала
    initializeSignalPropertyList(); // Инициализация листа со свойствами сигнала
    initializeAllPlot(); // Инициализация графических окон
    initializeShowParams(); // Инициализация параметров для отображения

    // Создание соединений сигнал - слот
    connect(ui->actionAddSignal, SIGNAL(triggered()), this, SLOT(addSignal())); // Добавить сигнал
    connect(ui->actionRemoveSignal, SIGNAL(triggered()), this, SLOT(removeSignal())); // Удалить сигнал
    connect(ui->actionSaveSignal, SIGNAL(triggered()), this, SLOT(saveSignal())); // Удалить сигнал
    connect(ui->listFile, SIGNAL(itemSelectionChanged()), this, SLOT(setSignalProperty()), Qt::QueuedConnection); // Установка свойств сигнала
    connect(ui->actionNewProject, SIGNAL(triggered()), this, SLOT(clearProject())); // Новый проект
    connect(ui->tableFileProperty, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(setColor(int, int))); // Установка цвета сигнала
    connect(ui->spinBoxTimeWidth, SIGNAL(editingFinished()), this, SLOT(setTimeWindowProperty())); // Установка параметров окна по ширине
    connect(ui->spinBoxOverlapFactor, SIGNAL(editingFinished()), this, SLOT(setTimeWindowProperty())); // Установка параметров окна по коэффициенту перекрытия
    connect(ui->spinBoxShowWindow, SIGNAL(valueChanged(int)), this, SLOT(setShowParams())); // Установка параметров отображения
}

// Деструктор главного окна
MainWindow::~MainWindow()
{
    delete ui;
}
