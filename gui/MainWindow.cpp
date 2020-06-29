#include "MainWindow.h"
#include "ui_MainWindow.h"

// Конструктор главного окна
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    statSignal_(vecDataSignal_, widthTimeWindow_, shiftWindow_, 1, 2) // Статистики
{
    ui->setupUi(this); // Инициализация графического интерфейса
    signalCharacteristicsWindow_ = QSharedPointer<SignalCharacteristicsWindow>(new SignalCharacteristicsWindow(calcTemplate_, this));
    levelsWindow_ = QSharedPointer<LevelsWindow>(new LevelsWindow(calcTemplate_, vecDataSignal_, this));
    associatedStatisticsWindow_ = QSharedPointer<AssociatedStatisticsWindow>(new AssociatedStatisticsWindow(calcTemplate_, vecDataSignal_, this));
    calcTemplateWindow_ = QSharedPointer<CalculationTemplateWindow>(new CalculationTemplateWindow(calcTemplate_, vecDataSignal_, this));
    filterSignalsWindow_ = QSharedPointer<FilterSignalsWindow>(new FilterSignalsWindow(vecDataSignal_, this));
    initializeCalculationParams(); // Выставление расчетных параметров
    clearSignalPropertyList(); // Очистка листа со свойствами сигнала
    initializeSignalPropertyList(); // Инициализация листа со свойствами сигнала
    initializeAllPlot(); // Инициализация графических окон
    initializeShowParams(); // Инициализация параметров для отображения

    // Создание соединений сигнал - слот
    connect(ui->actionAddSignal, SIGNAL(triggered()), this, SLOT(addSignal())); // Добавить сигнал
    connect(ui->actionRemoveSignal, SIGNAL(triggered()), this, SLOT(removeSignal())); // Удалить сигнал
    connect(ui->actionSaveSignal, SIGNAL(triggered()), this, SLOT(saveSignalCharacteristics())); // Сохранить сигнал
    connect(ui->actionFilterSignals, SIGNAL(triggered()), this, SLOT(filterSignals())); // Фильтрация сигналов
    connect(ui->actionSaveCalculation, SIGNAL(triggered()), this, SLOT(saveCalcualtion())); // Сохранить результаты расчета
    connect(ui->actionSaveLevels, SIGNAL(triggered()), this, SLOT(saveLevels())); // Сохранить разбиения по уровням
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
    connect(ui->comboBoxRegression, SIGNAL(currentIndexChanged(int)), this, SLOT(updateRegressionState())); // Проверка возможности построения линейной регрессии
    connect(ui->pushButtonShowRegression, SIGNAL(clicked()), this, SLOT(plotRegression())); // Отобразить рассеяние для пары сигналов
    connect(ui->actionSaveScreenshot, SIGNAL(triggered()), this, SLOT(saveScreenshot())); // Сохранить скриншот программы
    connect(ui->actionSaveAssociatedStatistics, SIGNAL(triggered()), this, SLOT(saveAssociatedStatistics())); // Сохранить относительные статистики
    connect(ui->actionAddShiftSignal, SIGNAL(triggered()), this, SLOT(addShiftSignal())); // Добавление сигнала со смещением
    connect(ui->actionCalculationTemplate, SIGNAL(triggered()), this, SLOT(changeCalculationTemplate())); // Добавление сигнала со смещением
    // Характеристики сигналов
    connect(ui->listFile, SIGNAL(itemSelectionChanged()), this, SLOT(updateSettingsOfCharacterstics()), Qt::QueuedConnection); // Проверяем возможность расчета
    connect(ui->spinBoxSpectrumWeightWindowWidth, SIGNAL(editingFinished()), this, SLOT(checkSpectrumWeightWindowWidth())); // При изменении ширины весового окна спектра
    connect(ui->spinBoxIntegralWeightWindowWidth, SIGNAL(editingFinished()), this, SLOT(checkIntegralWeightWindowWidth())); // При изменении ширины весового окна интеграла
    connect(ui->pushButtonSpectrumCalculate, SIGNAL(clicked()), this, SLOT(calculateAndPlotSpectrum())); // Расчет и построение спектра
    connect(ui->pushButtonIntegralCalculate, SIGNAL(clicked()), this, SLOT(calculateAndPlotIntegral())); // Расчет и построение интеграла
    connect(ui->pushButtonAnalysisCalculate, SIGNAL(clicked()), this, SLOT(calculateAndPlotAnalysis())); // Расчет и построение анализа
    connect(ui->checkBoxIntegralCorrection, SIGNAL(stateChanged(int)), this, SLOT(setEnabledIntegralCorrection())); // Установка состояния коррекции интеграла
    connect(ui->comboBoxIntegralDomain, SIGNAL(currentIndexChanged(int)), this, SLOT(setEnabledIntegralDomain())); // Установка состояния параметров интегрирования
    connect(ui->pushButtonSpectrumSave, SIGNAL(clicked()), this, SLOT(saveCharacteristic())); // Сохранение спектра
    connect(ui->pushButtonIntegralSave, SIGNAL(clicked()), this, SLOT(saveCharacteristic())); // Сохранение интеграла
    connect(ui->pushButtonAnalysisSave, SIGNAL(clicked()), this, SLOT(saveCharacteristic())); // Сохранение анализа
    // Обновление statusBar
    connect(ui->showModeWidget, SIGNAL(currentChanged(int)), this, SLOT(updateStatusBar())); // При переключении типа графиков
    connect(ui->listFile, SIGNAL(itemSelectionChanged()), this, SLOT(updateStatusBar()), Qt::QueuedConnection); // При выборе сигнала
    // Дополнительные окна
    connect(signalCharacteristicsWindow_.data(), SIGNAL(accepted()), this, SLOT(saveSignalCharacteristicsFinished())); // Сохранение характеристик
    connect(levelsWindow_.data(), SIGNAL(accepted()), this, SLOT(saveLevelsFinished())); // Сохранение уровней
    connect(associatedStatisticsWindow_.data(), SIGNAL(accepted()), this, SLOT(saveAssociatedStatisticsFinished())); // Сохранение относительных статистик
    connect(calcTemplateWindow_.data(), SIGNAL(finished(int)), this, SLOT(calculationTemplateProcessed(int))); // Сохранение расчетного шаблона
    connect(calcTemplateWindow_.data(), SIGNAL(apply(QVector<int>, int, int, int)), this, SLOT(applyCalculationTemplate(QVector<int>, int, int, int))); // Применение расчетного шаблона
    connect(filterSignalsWindow_.data(), SIGNAL(accepted()), this, SLOT(filtrationFinished())); // Фильтрация сигналов завершена
    // Справка
    connect(ui->actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(ui->actionAboutProgram, SIGNAL(triggered()), this, SLOT(aboutProgram()));
}

// Деструктор главного окна
MainWindow::~MainWindow()
{
    delete ui;
}
