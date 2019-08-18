#include "LevelsWindow.h"
#include "ui_LevelsWindow.h"

LevelsWindow::LevelsWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LevelsWindow)
{
    ui->setupUi(this);
}

LevelsWindow::~LevelsWindow()
{
    delete ui;
}
