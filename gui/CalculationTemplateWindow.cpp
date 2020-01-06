#include <QFileDialog>
#include <QFileInfo>
#include "CalculationTemplateWindow.h"
#include "ui_CalculationTemplateWindow.h"

static QString const TEMPLATE_EXTENSION = ".est";
// Конструктор
CalculationTemplateWindow::CalculationTemplateWindow(CalculationTemplate & calcTemplate, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CalculationTemplateWindow),
    pCalcTemplate_(&calcTemplate)
{
    ui->setupUi(this);
    // Создание соединений сигнал - слот
    connect(ui->pushButtonRecord, SIGNAL(clicked()), this, SLOT(changeState())); // Изменение состояния записи шаблона
    connect(ui->pushButtonRemoveWindow, SIGNAL(clicked()), this, SLOT(removeAction())); // Удаление действия
    connect(ui->pushButtonSave, SIGNAL(clicked()), this, SLOT(save())); // Сохранить шаблон

}

// Деструктор
CalculationTemplateWindow::~CalculationTemplateWindow()
{
    delete ui;
}

// Слоты

// Установить состояние записи
void CalculationTemplateWindow::changeState(){
    bool currentState = pCalcTemplate_->isRecord();
    if (!currentState){
        ui->pushButtonRecord->setText("Остановить запись");
        ui->pushButtonSave->setEnabled(false);
    }
    else {
        ui->pushButtonRecord->setText("Начать запись");
        if (!pCalcTemplate_->isEmpty())
            ui->pushButtonSave->setEnabled(true);
    }
    pCalcTemplate_->setStateRecord(!currentState);
}

// Обновить последовательность действий
void CalculationTemplateWindow::updateSequenceOfActions(){
    QList<QString> const& sequenceActions = pCalcTemplate_->getSequenceOfWindows();
    ui->listWidgetSequenceOfWindows->clear();
    QString tString = "";
    for (QString const& action : sequenceActions){
        if (!action.compare("SignalCharacteristicsWindow"))
            tString = "Характеристики сигнала";
        ui->listWidgetSequenceOfWindows->addItem(tString);
    }
}

// Установка пути по умолчанию
void CalculationTemplateWindow::setLastPath(QString const& lastPath){
    lastPath_ = lastPath;
}

// Удалить действие
void CalculationTemplateWindow::removeAction(){
    pCalcTemplate_->removeWindow(ui->listWidgetSequenceOfWindows->currentRow());
    updateSequenceOfActions();
}

// Сохранить шаблон
void CalculationTemplateWindow::save(){
    // Диалог с пользователем для выбора файла
    QString fileFullPath = QFileDialog::getSaveFileName(this, "Сохранение расчетного шаблона", lastPath_, "Расчетный шаблон программы EvalSig (*" + TEMPLATE_EXTENSION + ")"); // Диалоговое окно
    QFileInfo fileInfo(fileFullPath);
    // Проверка корректности выбора
    if (fileFullPath.isEmpty()) return;
    lastPath_ = fileInfo.absolutePath() + QDir::separator(); // Запись последней директории
    QString fileName = fileInfo.baseName() + TEMPLATE_EXTENSION;
    int exitStatus = pCalcTemplate_->write(lastPath_, fileName);
    if (!exitStatus) emit this->accepted();
}

// Прочитать шаблон из файла
void CalculationTemplateWindow::read(){
    // Диалог с пользователем для выбора файла
    QString fileFullPath = QFileDialog::getOpenFileName(this, "Загрузить расчетный шаблон", lastPath_, "Расчетный шаблон программы EvalSig (*" + TEMPLATE_EXTENSION + ")"); // Диалоговое окно
    QFileInfo fileInfo(fileFullPath);
    // Проверка корректности выбора
    if (fileFullPath.isEmpty()) return;
    lastPath_ = fileInfo.absolutePath() + QDir::separator(); // Запись последней директории
    // ---> TODO <--- Проверка корректности расширения
//    QString fileName = fileInfo.baseName() + TEMPLATE_EXTENSION;
//    int exitStatus = pCalcTemplate_->read(lastPath_, fileName);
//    if (!exitStatus) emit this->accepted();
}
