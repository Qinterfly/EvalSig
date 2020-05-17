#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "core/Statistics.h"
#include "gui/QCustomPlotZoom.h"
#include "gui/SignalCharacteristicsWindow.h"
#include "gui/LevelsWindow.h"
#include "gui/AssociatedStatisticsWindow.h"
#include "core/CalculationTemplate.h"
#include "gui/CalculationTemplateWindow.h"
#include "gui/FilterSignalsWindow.h"

enum ColorMapType { STATS, SPECTRUM, GROUP }; // Тип цветовой карты

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

public slots:
    // Чтение, запись и удаление сигналов
    void addSignal(int shiftRead = 0); // Добавить сигнал
    void addShiftSignal(); // Добавить сигнал со смещением
    void removeSignal(bool isReplot = true); // Удалить сигнал
    void saveSignalCharacteristics(); // Сохранить характеристики сигнала
    void saveCalcualtion(); // Сохранение результатов расчета
    void saveLevels(); // Сохранить разбиения по уровням
    void saveScreenshot(); // Сохранить скриншот программы
    void saveAssociatedStatistics(); // Сохранить относительные статистики
    void changeCalculationTemplate(); // Изменить расчетный шаблон
    void filterSignals(); // Фильтрация сигналов
    // Set методы
    void setSignalProperty(); // Установка свойств сигнала
    void setColor(int row, int column); // Установка цвета сигнала
    void setTimeWindowProperty(); // Установка параметров окна
    void setShowParams(); // Установка параметров отображения
    void setVisibleFileWidget(bool); // Изменить отображения списка сигналов
    void setVisiblePropertyWidget(bool); // Изменить отображения виджета свойств
    void setStatEstimationBoundaries(); // Установка границ расчета (поля -> статистики)
    void setStatEstimationBoundaries(QPair<int, int> const& estimationBoundaries); // Установка границ расчета (статистики -> поля)
    void changeDataSignal(int row, int column); // Изменение параметров сигналов
    void applyCalculationTemplate(QVector<int> iSelectedSignals, int iMainSignal, int iBaseSignal, int iSupportSignal); // Применение расчетного шаблона
    // Обновление
    void updateStatusBar(); // Обновление строки состояния
    // Методы очистки
    void clearProject(); // Очистка проекта
    void clearSignalCharacteristics(); // Очистка обработанных сигналов
    // Обмен данными между окнами
    void saveSignalCharacteristicsFinished(); // Завершение сохранения свойств сигнала
    void saveLevelsFinished(); // Завершение сохранение поуровневого разбиения
    void saveAssociatedStatisticsFinished(); // Завершение сохранения относительных статистик
    void calculationTemplateProcessed(int state); // Завершение сохранения расчетного шаблона
    void filtrationFinished(); // Фильтрация сигналов завершена
    // Справка
    void aboutProgram(); // Информация о программе
    // Регрессия
    void updateRegressionState(); // Проверка возможности построения линейной регрессии
    void plotRegression(); // Построение линейной регрессии
    void clearRegression(); // Очистка линейной регрессии
    // Расчет и построение характеристик сигналов
    void calculateAndPlotSpectrum(bool isPlot = true); // Спектра
    void calculateAndPlotIntegral(bool isPlot = true); // Интеграла
    void calculateAndPlotAnalysis(bool isPlot = true); // Анализа
    // Установка параметров характеристик сигналов
    void updateSettingsOfCharacterstics(); // Проверка возможности расчета и сохранения обработанных сигналов
    void checkSpectrumWeightWindowWidth(); // Проверка ширины весового окна спектра
    void checkIntegralWeightWindowWidth(); // Проверка ширины весового окна интеграла
    void setEnabledIntegralCorrection(); // Установка состояния коррекции интеграла
protected:
    bool eventFilter(QObject * obj, QEvent * event) override; // Переопределение событий программы
private:
    // Инициализация параметров программы
    void initializeSignalPropertyList(); // Инициализация листа со свойствами сигнала
    void initializeCalculationParams(); // Выставление расчетных параметров
    void initializeShowParams(); // Инициализация параметров для отображени
    void initializeAllPlot(); // Инициализация графических окон
    void initializeAllColorMap(); // Инициализация цветовых карт
    // Set методы
    void setBoundariesShowParams(); // Выставления границ отображения для объектов интерфейса
    // Методы очистки
    void clearSignalPropertyList(); // Очистка листа со свойствами сигнала
    void clearStatusBar(); // Очистка информационной строки
    // Работа с графиками
    void addGraph(bool isReplot = false); // Добавление графика
    void removeGraph(int deleteInd); // Удаление графика
    void replotGraph(int); // Обновление графика
    void replotAllGraphs(); // Обновление всех графиков
    void plotEstimationBoundaries(bool isReplot = false); // Построение расчетных границ
    void clearDataEstimationsBoundaries(); // Очистка данных графиков расчетных границ
    // Работа с цветовыми картами
    void plotAllColorMap(); // Построение цветовые карты
    void setColorMapData(int plotInd, int nGrid); // Выставление данных для цветовой карты по индексу
    void setTextTickerForStats(int plotInd, int nGrid); // Выставление подписей поверхности статистик
    void setTextTickerForSpectrum(int plotInd, int nGrid, QVector<bool> mask); // Выставление подписей поверхности спектров
    void setTextTickerForGroup(int plotInd, int nGrid); // Выставление подписей поверхности группы
    void assignDataForColorMap(ArrayRegressionParams const& arrData, int plotInd, int nGrid); // Назначение данных в ColorMap для ArrayRegressionParams
    void assignDataForColorMap(ArrayStatCharacters const& arrData, int plotInd, int nGrid); // Назначение данных в ColorMap для ArrayStatCharacters
    void assignDataForSpectrumSurface(int plotInd, int nGrid); // Назначение данных для поверхности спектров
    void assignDataForGroupSurface(int plotInd, int nGrid); // Назначение данных для групповой поверхности
    void clearAllColorMap(); // Очистка цветовых карт
private:
    Ui::MainWindow * ui; // Графический интерфейс QtDesigner
    QString lastPath_ = ""; // Последний путь, выбранный пользователем
    // Дополнительные окна
    SignalCharacteristicsWindow * signalCharacteristicsWindow_; // Окно сохранение характеристик сигнала
    LevelsWindow * levelsWindow_; // Окно сохранение разбиения по уровням
    AssociatedStatisticsWindow * associatedStatisticsWindow_; // Окно относительных статистик
    CalculationTemplateWindow * calcTemplateWindow_; // Окно построения расчетного шаблона
    FilterSignalsWindow * filterSignalsWindow_; // Окно фильтрации сигналов
    // Данные
    QVector<DataSignal> vecDataSignal_; // Вектор с исходными сигналами
    int widthTimeWindow_ = 0; // Ширина окна
    int shiftWindow_ = 0; // Смещение левой границы временного окна
    Statistics statSignal_; // Статистические характеристики для сигналов
    // Отображение
    int showWindow_; // Номер временного окна для показа
    QVector<QColor> colorList_; // Список цветов для отображения
    QVector<bool> availableColors_; // Вектор доступных цветов для выбора
    QLabel * calcStatusLabel; // Информация о расчете в statusBar
    int const SECONDARY_PLOT_IND = 2; // Число вспомогательных графиков
    QCPColorScale * colorScaleRegression_; // Цветовая карта регрессии
    // Контейнеры для построения цветовых карт
    QVector<QCustomPlotZoom *> vecTablePlot_; // Вектор указателей на графические объекты
    QVector<QCPColorMap *> vecColorMap_; // Вектор указателей на цветовые карты
    QVector<int> vecColorMapType_; // Типы цветовых карт
    QVector<QCPColorScale *> vecColorScale_; // Вектор указателей на цветовые шкалы
    // Расчетный шаблон
    CalculationTemplate calcTemplate_;
    // Сигналы, полученные после применения численных методов
    QMap<int, DataSignal> mapSignalCharacteristics_;          // Данные
    QMap<int, QPointer<QPushButton>> mapCalculationButtons_;  // Расчетные кнопки
    QMap<int, QPointer<QPushButton>> mapSaveButtons_;         // Кнопки для сохранения
};

#endif // MAINWINDOW_H
