#ifndef FILTERSIGNALSWINDOW_H
#define FILTERSIGNALSWINDOW_H

#include <QDialog>
#include <QListWidget>
#include "core/DataSignal.h"

namespace Ui {
class FilterSignalsWindow;
}

class FilterSignalsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit FilterSignalsWindow(QVector<DataSignal> const& vecDataSignal, QWidget *parent = nullptr);
    ~FilterSignalsWindow();
    void setSignalsName(QListWidget const& listWidgetSignals); // Определение имен сигналов для выбора
    void setLastPath(QString const& lastPath); // Установка пути по умолчанию
    QString const& lastPath() { return lastPath_; } // Получение пути по умолчанию
private:
    void setTimeBoundaries(); // Задание временных границ
    void showEvent(QShowEvent * event) override; // При отображении виджета
private:
    Ui::FilterSignalsWindow *ui;
    QVector<DataSignal> const& vecDataSignal_; // Вектор с исходными сигналами
    QString lastPath_ = ""; // Последний путь, выбранный пользователем
};


#endif // FILTERSIGNALSWINDOW_H
