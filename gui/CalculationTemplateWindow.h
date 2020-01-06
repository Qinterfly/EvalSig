#ifndef TEMPLATEWINDOW_H
#define TEMPLATEWINDOW_H

#include <QDialog>
#include "gui/CalculationTemplate.h"

namespace Ui {
class CalculationTemplateWindow;
}

class CalculationTemplateWindow : public QDialog
{
    Q_OBJECT

public:
    explicit CalculationTemplateWindow(CalculationTemplate & calcTemplate, QWidget *parent = nullptr);
    ~CalculationTemplateWindow();
    QString const& lastPath() const { return lastPath_; }
public slots:
    void updateSequenceOfActions(); // Обновить последовательность действий
    void setLastPath(QString const& lastPath); // Установка пути по умолчанию
private slots:
    void changeState(); // Изменить состояние записи
    void removeAction(); // Удалить действие
    void save(); // Сохранить шаблон
    void read(); // Прочитать шаблон из файла
private:
    Ui::CalculationTemplateWindow *ui;
    CalculationTemplate * const pCalcTemplate_; // Указатель на расчетный шаблон
    QString lastPath_ = ""; // Последний путь, выбранный пользователем
};

#endif // TEMPLATEWINDOW_H
