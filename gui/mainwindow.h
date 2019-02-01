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
    // Работа с исходными сигналами
    void addSignal(); // Добавить сигнал
    void removeSignal(); // Удалить сигнал

public:
    // Работа с графиками
    void replot(); // Перерисовать график
    void initializeSignalPropertyList(); // Инициализация листа со свойствами сигнала
    void clearSignalPropertyList(); // Очистка листа со свойствами сигнала
    void setSystemParams(); // Установка параметров используемой операционной системы

private:
    Ui::MainWindow *ui; // Графический интерфейс QtDesigner
    QString lastPath_ = ""; // Последний путь, выбранный пользователем
    QString pathSymbol_ = "/"; // Символ разделения директорий
    // Данные
    QVector<DataSignal> vecDataSignal_; // Вектор с исходными сигналами
    int widthTimeWindow_ = 0; // Ширина окна
    double overlapFactor_ = 0; // Коэффициент перекрытия окон
    Statistics statSignal_; // Статистические характеристики для сигналов
};

#endif // MAINWINDOW_H
