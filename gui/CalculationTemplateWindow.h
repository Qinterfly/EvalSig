#ifndef TEMPLATEWINDOW_H
#define TEMPLATEWINDOW_H

#include <QDialog>
#include "core/CalculationTemplate.h"
#include "core/DataSignal.h"

namespace Ui {
class CalculationTemplateWindow;
}

class CalculationTemplateWindow : public QDialog
{
    Q_OBJECT

public:
    explicit CalculationTemplateWindow(CalculationTemplate & calcTemplate, QVector<DataSignal> const& vecDataSignal, QWidget *parent = nullptr);
    ~CalculationTemplateWindow();
    QString const& lastPath() const { return lastPath_; }
public slots:
    void updateSequenceOfWindows(); // Обновить последовательность окон
    void setLastPath(QString const& lastPath); // Установка пути по умолчанию
private slots:
    void changeState(); // Изменить состояние записи
    void removeWindow(); // Удалить окно
    void save(); // Сохранить шаблон
    void load(); // Загрузить шаблон
private:
    Ui::CalculationTemplateWindow *ui;
    CalculationTemplate & calcTemplate_; // Расчетный шаблон
    QVector<DataSignal> const& vecDataSignal_; // Временные сигналы
    QString lastPath_ = ""; // Последний путь, выбранный пользователем
};

#endif // TEMPLATEWINDOW_H
