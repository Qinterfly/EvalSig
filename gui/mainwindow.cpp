#include "mainwindow.h"
#include "ui_mainwindow.h"

// Конструктор главного окна
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

// Деструктор главного окна
MainWindow::~MainWindow()
{
    delete ui;
}
