#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
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
    connect(ui->pushButtonResultPath, SIGNAL(clicked()), this, SLOT(setResultPath())); // Установка пути сохранения результатов
    connect(ui->pushButtonApply, SIGNAL(clicked()), this, SLOT(wrapApplyingTemplate())); // Применение шаблона
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(checkCreatingFinished(int))); // Проверка завершения создания шаблона
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(checkSelectedSignals()), Qt::QueuedConnection); // Проверка выбранных сигналов
    // Оценка возможности применения шаблона
    connect(ui->pushButtonRemoveWindow, SIGNAL(clicked()), this, SLOT(checkApplicability()), Qt::QueuedConnection); // При нажатии кнопки удалить
    connect(ui->pushButtonLoad, SIGNAL(clicked()), this, SLOT(checkApplicability()), Qt::QueuedConnection); // При загрузке шаблона
    connect(ui->pushButtonRecord, SIGNAL(clicked()), this, SLOT(checkApplicability()), Qt::QueuedConnection); // При изменении состояния записи
    connect(ui->listWidgetSignals, SIGNAL(itemSelectionChanged()), this, SLOT(checkApplicability()), Qt::QueuedConnection); // При изменении состояния записи
    connect(ui->pushButtonResultPath, SIGNAL(clicked()), this, SLOT(checkApplicability()), Qt::QueuedConnection); // При изменении пути сохранения результатов
    connect(ui->comboBoxBaseSignal, SIGNAL(currentIndexChanged(int)), this, SLOT(checkApplicability()), Qt::QueuedConnection); // При изменении базового сигнала
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
    for (QString const& window : sequenceWindows){
        QString tString = "NoName";
        if ( !window.compare("SignalCharacteristicsWindow") )
            tString = "Характеристики сигнала";
        if ( !window.compare("StatisticsWindow") )
            tString = "Статистические коэффициенты";
        if ( !window.compare("AssociatedStatisticsWindow") )
            tString = "Относительные статистики";
        if ( !window.compare("LevelsWindow") )
            tString = "Разбиение по уровням";
        ui->listWidgetSequenceOfWindows->addItem(tString);
    }
    ui->listWidgetSequenceOfWindows->setCurrentRow(0);
}

// Установка пути по умолчанию
void CalculationTemplateWindow::setLastPath(QString const& lastPath){
    lastPath_ = lastPath;
}

// Определение имен сигналов для выбора
void CalculationTemplateWindow::setSignalsName(QListWidget const& listSignals){
    int nItem = listSignals.count();
    ui->listWidgetSignals->clear(); // Очистка списка
    ui->comboBoxMainSignal->clear();
    ui->comboBoxBaseSignal->clear();
    ui->comboBoxSupportSignal->clear();
    // Добавление пустых элементов
    ui->comboBoxBaseSignal->addItem("");
    ui->comboBoxSupportSignal->addItem("");
    // Добавление имен сигналов
    for (int i = 0; i != nItem; ++i){
        QString const& signalName = listSignals.item(i)->text();
        ui->listWidgetSignals->addItem(signalName); // Список сигналов
        ui->comboBoxMainSignal->addItem(signalName); // Основной сигнал
        ui->comboBoxBaseSignal->addItem(signalName); // Базовый сигнал
        ui->comboBoxSupportSignal->addItem(signalName); // Опорный сигнал
    }
    ui->listWidgetSignals->setCurrentRow(0);
}

// Удалить окно
void CalculationTemplateWindow::removeWindow(){
    if (ui->listWidgetSequenceOfWindows->count() == 0) return;
    calcTemplate_.removeWindow(ui->listWidgetSequenceOfWindows->currentRow());
    updateSequenceOfWindows(); // Обновить последовательность окон
    checkSelectedSignals(); // Проверить выбранные сигналы
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
    if (!exitStatus) emit this->finished(Code::Saved);
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
        emit this->finished(Code::Loaded);
        ui->lineEditPath->setText(fileFullPath); // Путь
        ui->tableInformation->setItem(0, 1, new QTableWidgetItem(QString::number(calcTemplate_.version()))); // Версия
        ui->tableInformation->setItem(1, 1, new QTableWidgetItem(calcTemplate_.date())); // Дата
        ui->tableInformation->setItem(2, 1, new QTableWidgetItem(calcTemplate_.note())); // Примечание
        updateSequenceOfWindows();
    }
}

// Установка пути для сохранения результатов
void CalculationTemplateWindow::setResultPath(){
    // Диалог с пользователем для выбора директории для сохранения
    QString saveDir = QFileDialog::getExistingDirectory(this, "", lastPath_, QFileDialog::ShowDirsOnly); // Диалоговое окно
    // Проверка корректности выбора
    if (saveDir.isEmpty()) return;
    lastPath_ = saveDir + QDir::separator(); // Запись последней директории
    ui->lineEditResultPath->setText(lastPath_); // Отображение пути
}

// Проверка применимость шаблона к загруженным данным
void CalculationTemplateWindow::checkApplicability(){
    bool isBaseEmpty = false;
    if ( ui->comboBoxBaseSignal->isEnabled() && ui->comboBoxBaseSignal->currentText().isEmpty() ) // Проверка наличия базового сигнала
        isBaseEmpty = true;
    if ( calcTemplate_.isEmpty() || ui->listWidgetSignals->currentRow() < 0 || ui->lineEditResultPath->text().isEmpty() || isBaseEmpty){
        ui->pushButtonApply->setEnabled(false);
        return;
    }
    ui->pushButtonApply->setEnabled(true);
}

// Проверка завершения создания шаблона
void CalculationTemplateWindow::checkCreatingFinished(int index){
    if (calcTemplate_.isRecord() && index > 0){
        ui->tabWidget->setCurrentIndex(0);
        QMessageBox messageBox;
        messageBox.setText("Завершите создание расчетного шаблона");
        messageBox.setIcon(QMessageBox::Warning);
        messageBox.exec();
    }
}

// Проверка выбранных сигналов
void CalculationTemplateWindow::checkSelectedSignals(){
    if (ui->tabWidget->currentIndex() != 2) return;
    // Основной сигнал
    bool isMainSignal = false;
    if ( calcTemplate_.contains("AssociatedStatisticsWindow") )
        isMainSignal = true;
    ui->comboBoxMainSignal->setEnabled(isMainSignal);
    // Базовый и опорный сигналы
    bool isEnableLevels = false;
    if ( calcTemplate_.contains("LevelsWindow") )
        isEnableLevels = true;
    ui->comboBoxBaseSignal->setEnabled(isEnableLevels);
    ui->comboBoxSupportSignal->setEnabled(isEnableLevels);
}

// Подготовка данных для применения шаблона
void CalculationTemplateWindow::wrapApplyingTemplate() {
    emit this->finished(Code::StartedApplying); // Сигнал о начале применения шаблона
    QList<QListWidgetItem *> const& selectedItems = ui->listWidgetSignals->selectedItems();
    QVector<int> iSelectedSignals(selectedItems.size());
    int k = 0;
    for (QListWidgetItem * item : selectedItems)
        iSelectedSignals[k++] = ui->listWidgetSignals->row(item);
    int iMainSignal = ui->comboBoxMainSignal->isEnabled() ? ui->comboBoxMainSignal->currentIndex() : -1; // Основной сигнал
    int iBaseSignal = ui->comboBoxBaseSignal->isEnabled() ? ui->comboBoxBaseSignal->currentIndex() : -1; // Базовый сигнал
    int iSupportSignal = ui->comboBoxSupportSignal->isEnabled() ? ui->comboBoxSupportSignal->currentIndex() : -1; // Опорный сигнал
    emit this->apply(iSelectedSignals, iMainSignal, iBaseSignal, iSupportSignal);
}
