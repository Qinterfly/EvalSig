#ifndef SIGNALCHARACTERISTICSWINDOW_H
#define SIGNALCHARACTERISTICSWINDOW_H

#include <QDialog>
#include "core/DataSignal.h"

namespace Ui {
class SignalCharacteristicsWindow;
}

class SignalCharacteristicsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SignalCharacteristicsWindow(QWidget *parent = nullptr);
    ~SignalCharacteristicsWindow();
public:
    void setDataSignal(DataSignal const& dataSignal); // Установка временного сигнала
    void setEstimationBoundaries(QPair <int, int> const& estimationBoundaries); // Установка расчетных границ
    void setLastPath(QString const& lastPath); // Установка пути по умолчанию
    QString const& lastPath() { return lastPath_; } // Получение пути по умолчанию
public slots:
    void saveCharacteristics(bool isUserCalc = true); // Сохранение выбранных характеристик
    void checkWeightWindowWidth(); // Проверка ширины весового окна
    void checkBandpassFrequencies(); // Проверка частот в полосе пропускания
    void reject() override; // Переопределение закрытия окна
private:
    void setBoundaries(); // Установка границ изменения параметров
private:
    Ui::SignalCharacteristicsWindow *ui;
    DataSignal const * pDataSignal_; // Указатель на временной сигнал
    QPair <int, int> const * pEstimationBoundaries_; // Указатель на границы расчета
    QString lastPath_ = ""; // Последний путь, выбранный пользователем
};

#endif // SIGNALCHARACTERISTICSWINDOW_H
