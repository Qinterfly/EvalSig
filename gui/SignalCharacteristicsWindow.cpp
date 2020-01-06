#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QtMath>
#include "SignalCharacteristicsWindow.h"
#include "ui_SignalCharacteristicsWindow.h"
#include "core/NumericalFunctions.h"

// ---- Свойства сохраняемого сигнала --------------------------------------------------------------------------

// Конструктор окна свойств сохраняемого сигнала
SignalCharacteristicsWindow::SignalCharacteristicsWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SignalCharacteristicsWindow)
{
    ui->setupUi(this);
    // Сигналы - слоты
    connect(ui->pushButtonSaveCharacteristics, SIGNAL(clicked()), this, SLOT(saveCharacteristics())); // При нажатии кнопки "Сохранение"
    connect(ui->spinBoxWeightWindowWidth, SIGNAL(editingFinished()), this, SLOT(checkWeightWindowWidth())); // При изменении ширины весового окна
        // При изменении частот в полосе пропускания
    connect(ui->spinBoxChoosedSignalLowerFrequency, SIGNAL(editingFinished()), this, SLOT(checkBandpassFrequencies())); // Нижней
    connect(ui->spinBoxChoosedSignalUpperFrequency, SIGNAL(editingFinished()), this, SLOT(checkBandpassFrequencies())); // Верхней
}

// Деструктор
SignalCharacteristicsWindow::~SignalCharacteristicsWindow()
{
    delete ui;
}

// Установка временного сигнала
void SignalCharacteristicsWindow::setDataSignal(DataSignal const& dataSignal){
    pDataSignal_ = &dataSignal; // Временной сигнал
    setBoundaries(); // Установка границ изменения параметров
}

// Установка расчетных границ
void SignalCharacteristicsWindow::setEstimationBoundaries(QPair <int, int> const& estimationBoundaries){
    pEstimationBoundaries_ = &estimationBoundaries;
}

// Установка пути по умолчанию
void SignalCharacteristicsWindow::setLastPath(QString const& lastPath){
    lastPath_ = lastPath;
}

// Установка расчетного шаблона
void SignalCharacteristicsWindow::setCalculationTemplate(CalculationTemplate & calcTemplate){
    pCalcTemplate_ = &calcTemplate;
}

// Установка границ изменения параметров
void SignalCharacteristicsWindow::setBoundaries(){
    // Ширина окна не должна превышать длину сигнала
    int nSignal = pDataSignal_->size(); // Длина сигнала
    int powDegree2 = previousPow2(nSignal);// Ближайшая степень двойки
    ui->spinBoxWeightWindowWidth->setMaximum( static_cast<int>(qPow(2, powDegree2)) );
    // Настройка нижней и верхней частот пропускания
    double maxFreq = pDataSignal_->nyquistFrequency(); // Максимально возможная частота в спектре
    ui->spinBoxChoosedSignalLowerFrequency->setMaximum(maxFreq); // Нижняя частота
    ui->spinBoxChoosedSignalUpperFrequency->setMaximum(maxFreq); // Верхняя частота
}

// Сохранение выбранных характеристик
void SignalCharacteristicsWindow::saveCharacteristics(){
    // Диалог с пользователем для выбора директории для сохранения
    QString saveDir = QFileDialog::getExistingDirectory(this, "", lastPath_, QFileDialog::ShowDirsOnly); // Диалоговое окно
    int saveStatus; // Код сохранения
    // Проверка корректности выбора
    if (saveDir.isEmpty()){
        saveStatus = -1;
        return;
    }
    lastPath_ = saveDir + QDir::separator(); // Запись последней директории
    QFileInfo fileInfo(pDataSignal_->getName()); // Информация о файле
    QString const& signalName = fileInfo.baseName(); // Базовое имя сигналаа
    // Выставление флагов расчета
    saveStatus = 0; // Флаг успешности сохранения
    bool isApproximation = ui->groupBoxApproximation->isChecked(); // Аппроксимации
    bool isIntegration = ui->groupBoxIntegration->isChecked(); // Интегрирования
    bool isPowerSpectralDensity = ui->groupBoxPowerSpectralDensity->isChecked(); // Спектрального разложения
    bool isChoosedSignal = ui->groupBoxChoosedSignal->isChecked(); // Сохранение выбранного сигнала
    // Срез сигнала по расчетной области
    DataSignal resDataSignal(*pDataSignal_, pEstimationBoundaries_->first - 1, pEstimationBoundaries_->second - 1); // -1 при переводе отсчетов в индексы
    // Аппроксимация
    if (isApproximation){
        double smoothFactor = ui->spinBoxApproximationSmoothFactor->value(); // Коэффициент сглаживания
        DataSignal approxDataSignal = approximateSmoothSpline(resDataSignal, smoothFactor); // Аппроксимация сигнала
        saveStatus += approxDataSignal.writeDataFile(lastPath_, signalName + "-Аппрок" + ".txt"); // Сохранение результата аппроксимации
    }
    // Интегрирование
    if (isIntegration){
        int integrationOrder = ui->spinBoxIntegrationOrder->value(); // Порядок интегрирования
        double correctionFactor = -1; // Корректировочный коэффициент
        // Проверка необходимости коррекции
        if (ui->checkBoxIntegrationCorrection->isChecked())
            correctionFactor = ui->spinBoxIntegrationCorrectionFactor->value();
        QVector<DataSignal> integrVecDataSignal = integrate(resDataSignal, integrationOrder, correctionFactor); // Интегрирование сигнала
        // Сохранение результатов интегрирования
        for (int i = 0; i != integrVecDataSignal.size(); ++i)
            saveStatus += integrVecDataSignal[i].writeDataFile(lastPath_, signalName + "-Интегр_" + QString::number(i + 1) + ".txt");
    }
    // Спектральное разложение
    if (isPowerSpectralDensity){
        WindowFunction windowFun = WindowFunction(ui->comboBoxWeightWindowType->currentIndex()); // Тип окна (HAMMING, HANN, BLACKMAN)
        int weightWindowWidth = ui->spinBoxWeightWindowWidth->value(); // Ширина весового окна
        double overlapFactor = ui->spinBoxOverlapFactor->value(); // Коэффициент перекрытия окон
        int lengthSpectrum =  ui->spinBoxSpectrumInterpolation->value(); // Число точек для интерполяции
        int windowSmoothWidth = ui->spinBoxSmoothWidth->value(); // Число точек для сглаживания
        DataSignal powerSpectralDensitySignal = computePowerSpectralDensity(resDataSignal, windowFun, weightWindowWidth, overlapFactor, lengthSpectrum, windowSmoothWidth); // Вычисление спектра сигнала
        saveStatus += powerSpectralDensitySignal.writeDataFile(lastPath_, signalName + "-Спектр" + ".txt"); // Сохранение результата спектрального разложения
    }
    // Сохранение выбранного сигнала
    if (isChoosedSignal){
        // Проверка необходимости коррекции
        if (ui->checkBoxChoosedSignalCorrectionFactor->isChecked()){
            double correctionFactor = ui->spinBoxChoosedSignalCorrectionFactor->value(); // Коэффициент коррекции
            DataSignal corrDataSignal = correct(resDataSignal, correctionFactor); // Скорректированный временной сигнала
            saveStatus += corrDataSignal.writeDataFile(lastPath_, signalName + "-Скоррект" + ".txt"); // Сохранение скорректированного временного сигнала
        }
        // Проверка необходимости фильтрации
        if (ui->checkBoxChoosedSignalFiltration->isChecked()){
            WindowFunction windowFun = WindowFunction(ui->comboBoxWeightWindowType->currentIndex()); // Тип окна (HAMMING, HANN, BLACKMAN)
            int weightWindowWidth = ui->spinBoxWeightWindowWidth->value(); // Ширина весового окна
            double overlapFactor = ui->spinBoxOverlapFactor->value(); // Коэффициент перекрытия окон
            double lowerFreq = ui->spinBoxChoosedSignalLowerFrequency->value(); // Нижняя частота
            double upperFreq = ui->spinBoxChoosedSignalUpperFrequency->value(); // Верхняя частота
            DataSignal filterDataSignal = bandpassFilter(resDataSignal, windowFun, weightWindowWidth, overlapFactor, {lowerFreq, upperFreq});
            saveStatus += filterDataSignal.writeDataFile(lastPath_, signalName + "-Фильтр" + ".txt"); // Сохранение фильтрованного временного сигнала
        }
        saveStatus += resDataSignal.writeDataFile(lastPath_, signalName + ".txt"); // Сохранение исходного временного сигнала
    }
    if (saveStatus == 0) // В случае успешного сохранения
        emit this->accepted();
    this->hide(); // Скрытие окна
    // Заполнение расчетного шаблона
    if ( saveStatus != 0 || !pCalcTemplate_ || !pCalcTemplate_->isRecord() ) return;
    static QString const WINDOW_NAME = "SignalCharacteristicsWindow";
    // Данные аппроксимации
    pCalcTemplate_->addWindowData(WINDOW_NAME, "isApproximation", isApproximation); // Флаг аппроксимации
    pCalcTemplate_->addWindowData(WINDOW_NAME, "smoothFactor", ui->spinBoxApproximationSmoothFactor->value()); // Коэффициент сглаживания
    // Данные интегрирования
    pCalcTemplate_->addWindowData(WINDOW_NAME, "isIntegration", isIntegration); // Флаг интегрирования
    pCalcTemplate_->addWindowData(WINDOW_NAME, "integrationOrder", ui->spinBoxIntegrationOrder->value()); // Порядок интегрирования
    pCalcTemplate_->addWindowData(WINDOW_NAME, "isIntegrationCorrection", ui->checkBoxIntegrationCorrection->isChecked()); // Флаг коррекции
    pCalcTemplate_->addWindowData(WINDOW_NAME, "integrationCorrectionFactor", ui->spinBoxIntegrationCorrectionFactor->value()); // Корректировочный коэффициент
    // Данные спектрального разложения
    pCalcTemplate_->addWindowData(WINDOW_NAME, "isPowerSpectralDensity", isPowerSpectralDensity); // Флаг спектрального разложения
    pCalcTemplate_->addWindowData(WINDOW_NAME, "windowFun", ui->comboBoxWeightWindowType->currentIndex()); // Тип окна (HAMMING, HANN, BLACKMAN)
    pCalcTemplate_->addWindowData(WINDOW_NAME, "weightWindowWidth", ui->spinBoxWeightWindowWidth->value()); // Ширина весового окна
    pCalcTemplate_->addWindowData(WINDOW_NAME, "overlapFactor", ui->spinBoxOverlapFactor->value()); // Коэффициент перекрытия окон
    pCalcTemplate_->addWindowData(WINDOW_NAME, "lengthSpectrum", ui->spinBoxSpectrumInterpolation->value()); // Число точек для интерполяции
    pCalcTemplate_->addWindowData(WINDOW_NAME, "windowSmoothWidth", ui->spinBoxSmoothWidth->value()); // Число точек для сглаживания
    // Данные выбранного сигнала
    pCalcTemplate_->addWindowData(WINDOW_NAME, "isChoosedSignal", ui->groupBoxChoosedSignal->isChecked()); // Флаг сохранения выбранного сигнала
    pCalcTemplate_->addWindowData(WINDOW_NAME, "isChoosedSignalCorrection", ui->checkBoxChoosedSignalCorrectionFactor->isChecked()); // Флаг коррекции
    pCalcTemplate_->addWindowData(WINDOW_NAME, "choosedSignalCorrectionFactor", ui->spinBoxChoosedSignalCorrectionFactor->value()); // Коэффициент коррекции
    pCalcTemplate_->addWindowData(WINDOW_NAME, "isFiltration", ui->checkBoxChoosedSignalFiltration->isChecked()); // Флаг фильтрации
    pCalcTemplate_->addWindowData(WINDOW_NAME, "lowerFreq", ui->spinBoxChoosedSignalLowerFrequency->value()); // Нижняя частота
    pCalcTemplate_->addWindowData(WINDOW_NAME, "upperFreq", ui->spinBoxChoosedSignalUpperFrequency->value()); // Верхняя частота
}

// Проверка ширины весового окна
void SignalCharacteristicsWindow::checkWeightWindowWidth(){
    int weightWindowWidth = ui->spinBoxWeightWindowWidth->value(); // Текущая ширина окна
    int weightWindowWidth2 = static_cast<int>(qPow(2, previousPow2(weightWindowWidth))); // Ближайшая ширина окна, кратная двум
    // Установка ширины окна, кратной двум
    if (weightWindowWidth != weightWindowWidth2)
        ui->spinBoxWeightWindowWidth->setValue(weightWindowWidth2);
}

// Проверка частот в полосе пропускания
void SignalCharacteristicsWindow::checkBandpassFrequencies(){
    double lowerFreq = ui->spinBoxChoosedSignalLowerFrequency->value(); // Нижняя частота
    double upperFreq = ui->spinBoxChoosedSignalUpperFrequency->value(); // Верхняя частота
    // Проверка превышения нижней частоты значения верхней
    if ( lowerFreq > upperFreq ){
        ui->spinBoxChoosedSignalLowerFrequency->setValue(upperFreq);
        ui->spinBoxChoosedSignalUpperFrequency->setValue(lowerFreq);
    }
}

// Переопределение закрытия окна
void SignalCharacteristicsWindow::reject(){
    this->hide(); // Простое скрытие (для удержания настроек)
}

// -------------------------------------------------------------------------------------------------------------
