#ifndef SIGNALCHARACTERISTICSWINDOW_H
#define SIGNALCHARACTERISTICSWINDOW_H

#include <QDialog>
#include <QSharedPointer>
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
    void setLastPath(QString const& lastPath); // Установка пути по умолчанию
    QString const& lastPath() { return lastPath_; } // Получение пути по умолчанию
public slots:
    void saveCharacteristics(); // Сохранение выбранных характеристик
    void checkWeightWindowWidth(); // Проверка ширины весового окна
    void reject() override; // Переопределение закрытия окна
private:
    void setBoundaries(); // Установка границ изменения параметров
private:
    Ui::SignalCharacteristicsWindow *ui;
    DataSignal const * pDataSignal_; // Указатель на временной сигнал
    QString lastPath_ = ""; // Последний путь, выбранный пользователем
};

#endif // SIGNALCHARACTERISTICSWINDOW_H
