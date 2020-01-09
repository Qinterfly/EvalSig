#ifndef DOUBLEPROGRESSDIALOG_H
#define DOUBLEPROGRESSDIALOG_H

#include <QDialog>

// Окно для отображения текущего и общего прогресса
namespace Ui {
class DoubleProgressDialog;
}

class DoubleProgressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DoubleProgressDialog(QString const& name, QWidget *parent = nullptr);
    ~DoubleProgressDialog();
    // Пользовательский методы
    // Границы прогресса
    void setCurrentBoundaries(int min, int max);
    void setOverallBoundaries(int min, int max);
    // Текущий прогресс
    void setCurrentValue(int value);
    void setOverallValue(int value);
    // Текущие действия
    void setCurrentLabel(QString const& label);
    void setOverallLabel(QString const& label);
    // Состояние
    bool wasCanceled() const { return canceled_; }
private:
    Ui::DoubleProgressDialog *ui;
    bool canceled_ = false;
};

#endif // DOUBLEPROGRESSDIALOG_H
