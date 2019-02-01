#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "core/DataSignal.h"
#include "core/Statistics.h"

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
    // Чтение / запись сигналов
    void addSignal(); // Добавить сигнал
    void removeSignal(); // Удалить сигнал

public:
    // Инициализация параметров программы
    void initializeSignalPropertyList(); // Инициализация листа со свойствами сигнала
    void initializeSystemParams(); // Установка параметров используемой операционной системы
    void initializeCalculationParams(); // Выставление расчетных параметров
    void initializeShowParams(); // Инициализация параметров для отображения
    // Методы очистки
    void clearSignalPropertyList(); // Очистка листа со свойствами сигнала
    // Работа с графиками
    void replot(); // Перерисовать графики

private:
    Ui::MainWindow *ui; // Графический интерфейс QtDesigner
    QString lastPath_ = ""; // Последний путь, выбранный пользователем
    QString pathSymbol_ = "/"; // Символ разделения директорий
    // Данные
    QVector<DataSignal> vecDataSignal_; // Вектор с исходными сигналами
    int widthTimeWindow_ = 0; // Ширина окна
    double overlapFactor_ = 0; // Коэффициент перекрытия окон
    Statistics statSignal_; // Статистические характеристики для сигналов
    // Отображение
    int showWindow_; // Номер временного окна для показа
    const QStringList colorList_; // Список цветов для отображения
};

#endif // MAINWINDOW_H
