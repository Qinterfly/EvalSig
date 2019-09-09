#ifndef LEVELSWINDOW_H
#define LEVELSWINDOW_H

#include <QDialog>
#include <QListWidget>
#include "core/DataSignal.h"

namespace Ui {
class LevelsWindow;
}

class LevelsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit LevelsWindow(QVector<DataSignal> const& vecDataSignal, QWidget *parent = nullptr);
    ~LevelsWindow();
    void setSignalsName(QListWidget const& listSignals); // Определение имен сигналов для выбора
    void setEstimationBoundaries(QPair<int, int> const& estimationBoundaries); // Установка расчетных границ
    QString const& lastPath() { return lastPath_; } // Получение пути по умолчанию
private slots:
    void setSaveState(int); // Проверка возможности сохранения
    void save(); // Сохранение и расчет
    void showLevels(); // Отображение уровней на графике
private:
    void plot(QVector<double> const& X, QVector<double> const& Y, QPen penPlot); // Построение графика
    void clearAllPlot(); // Очистка всех графиков
    void setWidgetSizes(); // Установка размеров виджетов
private:
    Ui::LevelsWindow *ui;
    QVector<DataSignal> const& vecDataSignal_; // Вектор с исходными сигналами
    QString lastPath_ = ""; // Последний путь, выбранный пользователем
    QPair<int, int> estimationBoundaries_; // Расчетные границы
    QPair<int, int> calculationInd_;       // Индексы границ
};

#endif // LEVELSWINDOW_H
