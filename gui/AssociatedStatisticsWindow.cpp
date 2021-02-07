#include <QFileDialog>
#include "AssociatedStatisticsWindow.h"
#include "ui_AssociatedStatisticsWindow.h"

// ---- Интерфейс относительных статистик ----------------------------------------------------------------------

static QString const& WINDOW_NAME = "AssociatedStatisticsWindow";

// Конструктор
AssociatedStatisticsWindow::AssociatedStatisticsWindow(QVector<DataSignal> const& vecDataSignal, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AssociatedStatisticsWindow),
    vecDataSignal_(vecDataSignal)
{
    ui->setupUi(this);
    ui->tableNumberOfWindows->horizontalHeader()->setStretchLastSection(true); // Последняя колонка адаптируется под размер виджета
    // Создание соединений сигнал-слот
    connect(ui->pushButtonSave, SIGNAL(clicked()), this, SLOT(save())); // Сохранение результатов
        // Обновление числа окон
    connect(ui->comboBoxIndMainSignal, SIGNAL(activated(int)), this, SLOT(refreshNumberOfWindows()));
    connect(ui->spinBoxWidthWindow, SIGNAL(editingFinished()), this, SLOT(refreshNumberOfWindows()));  // При изменении ширины окна
    connect(ui->spinBoxShiftMainWindow, SIGNAL(editingFinished()), this, SLOT(refreshNumberOfWindows()));  // При изменении смещения главного окна
    connect(ui->spinBoxShiftCompareWindow, SIGNAL(editingFinished()), this, SLOT(refreshNumberOfWindows()));  // При изменении смещения сравниваемого окна
}

// Деструктор
AssociatedStatisticsWindow::~AssociatedStatisticsWindow()
{
    delete ui;
}

// Установка пути по умолчанию
void AssociatedStatisticsWindow::setLastPath(QString const& lastPath){
    lastPath_ = lastPath;
}

// Определение имен сигналов для выбора
void AssociatedStatisticsWindow::setSignalsName(QListWidget const& listSignals){
    int nItem = listSignals.count();
    // Очистка контейнеров
    ui->comboBoxIndMainSignal->clear();
    ui->tableNumberOfWindows->clearContents();
    // Добавление имен сигналов
    for (int i = 0; i != nItem; ++i){
        QString const& tName = listSignals.item(i)->text();
        ui->comboBoxIndMainSignal->addItem(tName);
        // В таблицу
        QTableWidgetItem * item = new QTableWidgetItem(tName);
        ui->tableNumberOfWindows->insertRow(i);
        ui->tableNumberOfWindows->setItem(i, 0, item);
    }
    ui->comboBoxIndMainSignal->setCurrentIndex(0); // Выбор первого элемента
}

// Выставление границ параметров окна
void AssociatedStatisticsWindow::setParamsBoundaries(){
    int nSignal = vecDataSignal_.size(); // Число сигналов
    int indMainSignal = ui->comboBoxIndMainSignal->currentIndex(); // Индекс главного сигнала
    int sizeMainSignal = vecDataSignal_[indMainSignal].size(); // Длина главного сигнала
    int sizeCompareSignal = 0; // Длина сравниваемого сигнала
    bool isGetFirstComp = true; // Флаг взятия длины первого сравниваемого сигнала
    // Отыскание минимальной длины сравниваемого сигнала
    for (int i = 0; i != nSignal; ++i){
        if (i == indMainSignal) continue;
        int tSize = vecDataSignal_[i].size();
        if (tSize < sizeCompareSignal || isGetFirstComp){
            sizeCompareSignal = tSize;
            isGetFirstComp = false;
        }
    }
    // Выставление границ
    ui->spinBoxWidthWindow->setMaximum(qMin(sizeMainSignal, sizeCompareSignal)); // Максимальная ширина окна
    ui->spinBoxShiftMainWindow->setMaximum(sizeMainSignal); // Смещение главного окна
    ui->spinBoxShiftCompareWindow->setMaximum(sizeCompareSignal); // Смещение окна для сравнения
}

// Обновление информации о числе окон по сигналам
void AssociatedStatisticsWindow::refreshNumberOfWindows(){
    int nSignal = vecDataSignal_.size(); // Число сигналов
    QVector<int> vecNumberOfWindows = AssociatedStatistics::calcNumberOfWindows(vecDataSignal_, ui->spinBoxWidthWindow->value(),  ui->spinBoxShiftMainWindow->value(),
                                             ui->spinBoxShiftCompareWindow->value(), ui->comboBoxIndMainSignal->currentIndex()); // Число окон по сигналам
    for (int i = 0; i != nSignal; ++i){
        QTableWidgetItem * item = new QTableWidgetItem(QString::number(vecNumberOfWindows[i]));
        ui->tableNumberOfWindows->setItem(i, 1, item);
    }
}

// Сохранение статистик
void AssociatedStatisticsWindow::save(bool isUserCalc){
    int exitStatus = 0; // Статус завершения
    // Диалог с пользователем для выбора директории для сохранения
    if (isUserCalc){
        QString saveDir = QFileDialog::getExistingDirectory(this, "", lastPath_, QFileDialog::ShowDirsOnly); // Диалоговое окно
        // Проверка корректности выбора
        if (saveDir.isEmpty()) return;
        lastPath_ = saveDir + QDir::separator(); // Запись последней директории
    }
    // Создание и расчет статистик
    AssociatedStatistics statistics(vecDataSignal_, ui->spinBoxWidthWindow->value(),  ui->spinBoxShiftMainWindow->value(),
                                    ui->spinBoxShiftCompareWindow->value(), ui->comboBoxIndMainSignal->currentIndex());
    exitStatus += statistics.computeStatistics(); // Полный расчет
    // Сохранение всех статистик
    exitStatus += statistics.writeAllStatistics(lastPath_);
    if (!exitStatus && isUserCalc) emit this->accepted();
    this->hide(); // Скрытие окна
}

// -------------------------------------------------------------------------------------------------------------
