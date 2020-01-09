#include "DoubleProgressDialog.h"
#include "ui_DoubleProgressDialog.h"

// ---- Окно для отображения текущего и общего прогресса -------------------------------------------------------

DoubleProgressDialog::DoubleProgressDialog(QString const& name, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DoubleProgressDialog)
{
    ui->setupUi(this);
    setWindowTitle(name);
}

DoubleProgressDialog::~DoubleProgressDialog()
{
    delete ui;
}

// Пользовательский методы

// Границы прогресса
void DoubleProgressDialog::setCurrentBoundaries(int min, int max){
    ui->progressBarCurrent->setRange(min, max);
}
void DoubleProgressDialog::setOverallBoundaries(int min, int max){
    ui->progressBarOverall->setRange(min, max);
}

// Текущий прогресс
void DoubleProgressDialog::setCurrentValue(int value){
    ui->progressBarCurrent->setValue(value);
}

void DoubleProgressDialog::setOverallValue(int value){
    ui->progressBarOverall->setValue(value);
}

// Текущие действия
void DoubleProgressDialog::setCurrentLabel(QString const& label){
    ui->labelCurrent->setText(label);
}
void DoubleProgressDialog::setOverallLabel(QString const& label){
    ui->labelOverall->setText(label);
}

// -------------------------------------------------------------------------------------------------------------
