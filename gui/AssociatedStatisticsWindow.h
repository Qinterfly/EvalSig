#ifndef ASSOCIATEDSTATISTICSWINDOW_H
#define ASSOCIATEDSTATISTICSWINDOW_H

#include <QDialog>
#include <QListWidget>
#include "core/DataSignal.h"
#include "core/AssociatedStatistics.h"
#include "core/CalculationTemplate.h"

// Интерфейс относительных статистик
namespace Ui {
class AssociatedStatisticsWindow;
}

class AssociatedStatisticsWindow : public QDialog
{
    Q_OBJECT
public:
    explicit AssociatedStatisticsWindow(CalculationTemplate & calcTemplate, QVector<DataSignal> const& vecDataSignal, QWidget *parent = nullptr);
    ~AssociatedStatisticsWindow();
    void applyCalculationTemplate(int mainIndex); // Применить расчетный шаблон
    void setLastPath(QString const& lastPath); // Установка пути по умолчанию
    void setSignalsName(QListWidget const& listSignals); // Определение имен сигналов для выбора
    void setParamsBoundaries(); // Выставление границ параметров окна
    QString const& lastPath() const { return lastPath_; } // Получение пути по умолчанию
public slots:
    void refreshNumberOfWindows(); // Обновление информации о числе окон по сигналам
    void save(bool isUserCalc = true); // Сохранение статистик
private:
    Ui::AssociatedStatisticsWindow *ui;
    CalculationTemplate & calcTemplate_;
    QVector<DataSignal> const& vecDataSignal_; // Вектор с исходными сигналами
    QString lastPath_ = ""; // Последний путь, выбранный пользователем
};

#endif // ASSOCIATEDSTATISTICSWINDOW_H
