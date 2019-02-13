
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "core/DataSignal.h"
#include "core/Statistics.h"
#include "include/QCustomPlot.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    // Чтение, запись и удаление сигналов
    void addSignal(); // Добавить сигнал
    void removeSignal(); // Удалить сигнал
    void saveSignal(); // Сохранить сигнал
    void saveCalcualtion(); // Сохранение результатов расчета
    // Set методы
    void setSignalProperty(); // Установка свойств сигнала
    void setColor(int row, int column); // Установка цвета сигнала
    void setTimeWindowProperty(); // Установка параметров окна
    void setShowParams(); // Установка параметров отображения
    void setVisibleFileWidget(bool); // Изменить отображения списка сигналов
    void setVisiblePropertyWidget(bool); // Изменить отображения виджета свойств
    // Обновление
    void updateStatusBar(); // Обновление строки состояния
    // Методы очистки
    void clearProject(); // Очистка проекта
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
    void setBoundaryShowParams(); // Выставления границ отображения для объектов интерфейса
    // Методы очистки
    void clearSignalPropertyList(); // Очистка листа со свойствами сигнала
    void clearStatusBar(); // Очистка информационной строки
    // Работа с графиками
    void addGraph(); // Добавление графика
    void removeGraph(int); // Удаление графика
    // Работа с цветовыми картами
    void plotAllColorMap(); // Построение цветовые карты
    void setColorMapData(int plotInd, int nGrid); // Выставление данных для цветовой карты по индексу
    void assignDataForColorMap(ArrayRegressionParams const& arrData, int plotInd, int nGrid); // Назначение данных в ColorMap для ArrayRegressionParams
    void assignDataForColorMap(ArrayStatCharacters const& arrData, int plotInd, int nGrid); // Назначение данных в ColorMap для ArrayStatCharacters
    void clearAllColorMap(); // Очистка цветовых карт
private:
    Ui::MainWindow * ui; // Графический интерфейс QtDesigner
    QString lastPath_ = ""; // Последний путь, выбранный пользователем
    // Данные
    QVector<DataSignal> vecDataSignal_; // Вектор с исходными сигналами
    int widthTimeWindow_ = 0; // Ширина окна
    int shiftWindow_ = 0; // Смещение левой границы временного окна
    Statistics statSignal_; // Статистические характеристики для сигналов
    // Отображение
    int showWindow_; // Номер временного окна для показа
    QVector<QColor> colorList_; // Список цветов для отображения
    QLabel * calcStatusLabel; // Информация о расчете в statusBar
    // Контейнеры для построения цветовых карт
    QVector<QCustomPlot *> vecTablePlot_; // Вектор указателей на графические обхъекты
    QVector<QCPColorMap *> vecColorMap_; // Вектор указателей на цветовые карты
    QVector<QCPColorScale *> vecColorScale_; // Вектор указателей на цветовые шкалы
};

#endif // MAINWINDOW_H
