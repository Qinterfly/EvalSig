#include "LevelsWindow.h"
#include "ui_LevelsWindow.h"
#include <QFileDialog>
#include "core/NumericalFunctions.h"
#include "core/DivisionDataSignal.h"

// Конструктор
LevelsWindow::LevelsWindow(QVector<DataSignal> const& vecDataSignal, QWidget *parent) :
    QDialog(parent), ui(new Ui::LevelsWindow), vecDataSignal_(vecDataSignal)
{
    ui->setupUi(this);
    // Создание соединений сигнал - слот
    connect(ui->comboBoxAccel, SIGNAL(currentIndexChanged(int)), this, SLOT(setSaveState(int))); // Установка возможности сохранения
    connect(ui->pushButtonSaveLevels, SIGNAL(clicked()), this, SLOT(save())); // Сохранение результатов
}

// Деструктор
LevelsWindow::~LevelsWindow()
{
    delete ui;
}

// Определение имен сигналов для выбора
void LevelsWindow::setSignalsName(QListWidget const& listSignals){
    int nItem = listSignals.count();
    // Очистка выпадающих списков
    ui->comboBoxAccel->clear();
    ui->comboBoxDisplacement->clear();
    // Добавление пустых элементов
    ui->comboBoxAccel->addItem("");
    ui->comboBoxDisplacement->addItem("");
    // Добавление имен сигналов в ускорения и перемещения
    for (int i = 0; i != nItem; ++i){
        QString const& tName = listSignals.item(i)->text();
        ui->comboBoxAccel->addItem(tName);
        ui->comboBoxDisplacement->addItem(tName);
    }
    // Выбор пустых элементов
    ui->comboBoxAccel->setCurrentIndex(0);
    ui->comboBoxDisplacement->setCurrentIndex(0);
}

// Установка возможности сохранения
void LevelsWindow::setSaveState(int){
    if (!ui->comboBoxAccel->currentText().isEmpty())
        ui->pushButtonSaveLevels->setEnabled(true);
    else
        ui->pushButtonSaveLevels->setEnabled(false);
}

// Сохранение и расчет
void LevelsWindow::save(){
    // Диалог с пользователем для выбора директории для сохранения
    QString saveDir = QFileDialog::getExistingDirectory(this, "", lastPath_, QFileDialog::ShowDirsOnly); // Диалоговое окно
    // Проверка корректности выбора
    if (saveDir.isEmpty()) return;
    lastPath_ = saveDir + QDir::separator(); // Запись последней директории
    // Сигналы
    DataSignal const& accel = vecDataSignal_[ui->comboBoxAccel->currentIndex() - 1]; // Ускорения
    DataSignal displacement; // Перемещения
    // Параметры
    double levelStep = ui->spinBoxLevelStep->value();
    double overlapFactor = ui->spinBoxOverlapFactor->value();
    double smoothIntegrFactor = ui->spinBoxSmoothIntegrFactor->value();
    double smoothApproxFactor = ui->spinBoxSmoothApproxFactor->value();
    double truncatePercent = ui->spinBoxTruncatePercent->value();
    double depthGluing = ui->spinBoxDepthGluing->value();
    // Получение перемещения
    if (ui->comboBoxDisplacement->currentText().isEmpty())
        displacement = integrate(accel, 2, smoothIntegrFactor)[1];
    else
        displacement = vecDataSignal_[ui->comboBoxDisplacement->currentIndex() - 1];
    // Вызов расчетных методов
    DivisionDataSignal divSignal(accel, displacement, levelStep, overlapFactor, smoothApproxFactor, truncatePercent, depthGluing,
                                    estimationBoundaries_.first, estimationBoundaries_.second);
    divSignal.calculateLevels(); // Расчет уровней
    // Сохранение результатов
    bool isPowerSpectralDensity = ui->groupBoxPowerSpectralDensity->isChecked(); // Спектрального разложения
        // Склейки
    divSignal.writeGluedParts(lastPath_);
        // Перемещения
    divSignal.writeDisplacement(lastPath_, "Перемещения.txt");
    divSignal.writeApproxDisplacement(lastPath_, "Аппр. перемещения.txt");
        // Спектры
    if (isPowerSpectralDensity){
        WindowFunction windowFun = WindowFunction(ui->comboBoxWeightWindowType->currentIndex()); // Тип окна (HAMMING, HANN, BLACKMAN)
        double spectrumOverlapFactor = ui->spinBoxSpectrumOverlapFactor->value(); // Коэффициент перекрытия окон
        int lengthSpectrum =  ui->spinBoxSpectrumInterpolation->value(); // Число точек для интерполяции
        int windowSmoothWidth = ui->spinBoxSmoothWidth->value(); // Число точек для сглаживания
        divSignal.calculatePowerSpectralDensity(windowFun, spectrumOverlapFactor, lengthSpectrum, windowSmoothWidth); // Вычисление спектров
        divSignal.writeSpectrum(lastPath_); // Сохранение спектров
    }
}

// Установка расчетных границ
void LevelsWindow::setEstimationBoundaries(QPair<int, int> const& estimationBoundaries){
    estimationBoundaries_ = estimationBoundaries;
}
