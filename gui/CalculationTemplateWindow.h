#ifndef TEMPLATEWINDOW_H
#define TEMPLATEWINDOW_H

#include <QDialog>
#include <QListWidget>
#include "core/CalculationTemplate.h"
#include "core/DataSignal.h"

// Интерфейс расчетного шаблона
namespace Ui {
class CalculationTemplateWindow;
}

class CalculationTemplateWindow : public QDialog
{
    Q_OBJECT
public:
    enum Code { Saved, Loaded, StartedApplying, FinishedApplying };
    explicit CalculationTemplateWindow(CalculationTemplate & calcTemplate, QVector<DataSignal> const& vecDataSignal, QWidget *parent = nullptr);
    ~CalculationTemplateWindow();
    QString const& lastPath() const { return lastPath_; }
signals:
    void apply(QVector<int> iSelectedSignals); // Применение шаблона к выбранным сигналам
public slots:
    void updateSequenceOfWindows(); // Обновить последовательность окон
    void setLastPath(QString const& lastPath); // Установка пути по умолчанию
    void setSignalsName(QListWidget const& listSignals); // Определение имен сигналов для выбора
private slots:
    void changeState(); // Изменить состояние записи
    void removeWindow(); // Удалить окно
    void save(); // Сохранить шаблон
    void load(); // Загрузить шаблон
    void setResultPath(); // Установка пути для сохранения результатов
    void checkApplicability(); // Проверка применимость шаблона к загруженным данным
    void checkCreatingFinished(int index); // Проверка завершения создания шаблона
    void wrapApplyingTemplate(); // Подготовка данных для применения шаблона
private:
    Ui::CalculationTemplateWindow *ui;
    CalculationTemplate & calcTemplate_; // Расчетный шаблон
    QVector<DataSignal> const& vecDataSignal_; // Временные сигналы
    QString lastPath_ = ""; // Последний путь, выбранный пользователем
};

#endif // TEMPLATEWINDOW_H
