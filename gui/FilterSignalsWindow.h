#ifndef FILTERSIGNALSWINDOW_H
#define FILTERSIGNALSWINDOW_H

#include <set>
#include <QDialog>
#include <QListWidget>
#include "core/DataSignal.h"

enum InterpolationType { LINEAR, SPLINE }; // Метод интерполяции

namespace Ui {
class FilterSignalsWindow;
}

class FilterSignalsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit FilterSignalsWindow(QVector<DataSignal> & vecDataSignal, QWidget *parent = nullptr);
    ~FilterSignalsWindow();
    void setSignalsName(QListWidget const& listWidgetSignals); // Определение имен сигналов для выбора
    void setLastPath(QString const& lastPath); // Установка пути по умолчанию
    QString const& lastPath() { return lastPath_; } // Получение пути по умолчанию
private:
    void setBoundaries(); // Задание границ данных
    void showEvent(QShowEvent * event) override; // При отображении виджета
    void keyPressEvent(QKeyEvent * event) override; // При нажатии клавиш
private slots:
    void checkStateFilter(); // Проверка возможности расчета
    void checkTimeBoundaries(); // Проверка временных границ
    void filterSignals(); // Фильтрация сигналов
    void setOutlierState(); // Установка состояние выбора предельного значения выбросов
    void setLinearFilterState(); // Установить состояния линейного фильтра
    void setScanPeriod(int newVal); // Установить период опроса
    void checkBandpassFrequencies(); // Проверка пользовательских частот для фильтрации
    void checkWeightWindowWidth(); // Проверка ширины весового окна
private:
    Ui::FilterSignalsWindow *ui;
    QVector<DataSignal> & vecDataSignal_; // Вектор с исходными сигналами
    QString lastPath_ = ""; // Последний путь, выбранный пользователем
    std::set<int> availableScanPeriods_; // Набор доступных для выбора периодов опроса
    int lastScanPeriod_; // Последний Период опроса
};


#endif // FILTERSIGNALSWINDOW_H
