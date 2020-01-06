#include <QFileDialog>
#include <QFileInfo>
#include "CalculationTemplateWindow.h"
#include "ui_CalculationTemplateWindow.h"

static QString const TEMPLATE_EXTENSION = "est";

// Конструктор
CalculationTemplateWindow::CalculationTemplateWindow(CalculationTemplate & calcTemplate, QVector<DataSignal> const& vecDataSignal, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CalculationTemplateWindow),
    calcTemplate_(calcTemplate),
    vecDataSignal_(vecDataSignal)
{
    ui->setupUi(this);
    // Создание соединений сигнал - слот
    connect(ui->pushButtonRecord, SIGNAL(clicked()), this, SLOT(changeState())); // Изменение состояния записи шаблона
    connect(ui->pushButtonRemoveWindow, SIGNAL(clicked()), this, SLOT(removeWindow())); // Удаление действия
    connect(ui->pushButtonSave, SIGNAL(clicked()), this, SLOT(save())); // Сохранить шаблон
    connect(ui->pushButtonLoad, SIGNAL(clicked()), this, SLOT(load())); // Загрузка шаблона
}

// Деструктор
CalculationTemplateWindow::~CalculationTemplateWindow()
{
    delete ui;
}

// Слоты

// Установить состояние записи
void CalculationTemplateWindow::changeState(){
    bool currentState = calcTemplate_.isRecord();
    if (!currentState){
        ui->pushButtonRecord->setText("Остановить запись");
        ui->pushButtonSave->setEnabled(false);
    }
    else {
        ui->pushButtonRecord->setText("Начать запись");
        if (!calcTemplate_.isEmpty())
            ui->pushButtonSave->setEnabled(true);
    }
    updateSequenceOfWindows();
    calcTemplate_.setStateRecord(!currentState);
}

// Обновить последовательность действий
void CalculationTemplateWindow::updateSequenceOfWindows(){
    // Информация о последовательности
    ui->labelLengthSequence->setText(QString::number(calcTemplate_.lengthSequence()));
    QList<QString> const& sequenceWindows = calcTemplate_.getSequenceOfWindows();
    ui->listWidgetSequenceOfWindows->clear();
    QString tString = "";
    for (QString const& window : sequenceWindows){
        if (!window.compare("SignalCharacteristicsWindow"))
            tString = "Характеристики сигнала";
        ui->listWidgetSequenceOfWindows->addItem(tString);
    }
}

// Установка пути по умолчанию
void CalculationTemplateWindow::setLastPath(QString const& lastPath){
    lastPath_ = lastPath;
}

// Удалить окно
void CalculationTemplateWindow::removeWindow(){
    if (ui->listWidgetSequenceOfWindows->count() == 0) return;
    calcTemplate_.removeWindow(ui->listWidgetSequenceOfWindows->currentRow());
    updateSequenceOfWindows();
}

// Сохранить шаблон
void CalculationTemplateWindow::save(){
    // Диалог с пользователем для выбора файла
    QString fileFullPath = QFileDialog::getSaveFileName(this, "Сохранение расчетного шаблона", lastPath_, "Расчетный шаблон программы EvalSig (*" + TEMPLATE_EXTENSION + ")"); // Диалоговое окно
    QFileInfo fileInfo(fileFullPath);
    // Проверка корректности выбора
    if (fileFullPath.isEmpty()) return;
    lastPath_ = fileInfo.absolutePath() + QDir::separator(); // Запись последней директории
    QString fileName = fileInfo.baseName() + "." + TEMPLATE_EXTENSION;
    calcTemplate_.setNote(ui->plainTextEditNote->toPlainText());
    int exitStatus = calcTemplate_.write(lastPath_, fileName);
    if (!exitStatus) emit this->finished(1);
}

// Загрузить шаблон из файла
void CalculationTemplateWindow::load(){
    // Диалог с пользователем для выбора файла
    QString fileFullPath = QFileDialog::getOpenFileName(this, "Загрузить расчетный шаблон", lastPath_, "Расчетный шаблон программы EvalSig (*." + TEMPLATE_EXTENSION + ")"); // Диалоговое окно
    QFileInfo fileInfo(fileFullPath);
    // Проверка корректности выбора
    if (fileFullPath.isEmpty() || fileInfo.completeSuffix().compare(TEMPLATE_EXTENSION)) return;
    lastPath_ = fileInfo.absolutePath() + QDir::separator(); // Запись последней директории
    QString fileName = fileInfo.fileName(); // Имя шаблона
    // Чтение шаблона
    int exitStatus = calcTemplate_.read(lastPath_, fileName);
    // Заполнение полей
    if (!exitStatus){
        emit this->finished(2);
        ui->lineEditPath->setText(fileFullPath); // Путь
        ui->tableInformation->setItem(0, 1, new QTableWidgetItem(QString::number(calcTemplate_.version()))); // Версия
        ui->tableInformation->setItem(1, 1, new QTableWidgetItem(calcTemplate_.date())); // Дата
        ui->tableInformation->setItem(2, 1, new QTableWidgetItem(calcTemplate_.note())); // Примечание
        updateSequenceOfWindows();
    }
}
