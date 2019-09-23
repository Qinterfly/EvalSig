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
    connect(ui->pushButtonShowLevels, SIGNAL(clicked()), this, SLOT(showLevels())); // Отображение уровней на графике
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

// Установка расчетных границ
void LevelsWindow::setEstimationBoundaries(QPair<int, int> const& estimationBoundaries){
    estimationBoundaries_ = estimationBoundaries;
    calculationInd_ = {estimationBoundaries_.first - 1, estimationBoundaries_.second - 1}; // Расчетные индексы
}

// Установка возможности сохранения
void LevelsWindow::setSaveState(int){
    if (!ui->comboBoxAccel->currentText().isEmpty()){
        ui->pushButtonSaveLevels->setEnabled(true);
        ui->pushButtonShowLevels->setEnabled(true);
    }
    else {
        ui->pushButtonSaveLevels->setEnabled(false);
        ui->pushButtonShowLevels->setEnabled(false);
        clearAllPlot();
    }
}

// Сохранение и расчет
void LevelsWindow::save(){
    // Диалог с пользователем для выбора директории для сохранения
    QString saveDir = QFileDialog::getExistingDirectory(this, "", lastPath_, QFileDialog::ShowDirsOnly); // Диалоговое окно
    // Проверка корректности выбора
    if (saveDir.isEmpty()) return;
    lastPath_ = saveDir + QDir::separator(); // Запись последней директории
    // Сигналы
    DataSignal accel = vecDataSignal_[ui->comboBoxAccel->currentIndex() - 1]; // Ускорения
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
    // Нормализация данных
    accel.normalize(FIRST); // Ускорений
    displacement.normalize(FIRST); // Перемещений
    DataSignal approxDisplacement = approximateSmoothSpline(displacement, smoothApproxFactor); // Аппроксимация перемещений
    // Вызов расчетных методов
    DivisionDataSignal divSignal(accel, displacement, approxDisplacement, levelStep, overlapFactor, truncatePercent, depthGluing,
                                    estimationBoundaries_.first, estimationBoundaries_.second);
    divSignal.calculateLevels(); // Расчет уровней
    // Сохранение результатов
    bool isPowerSpectralDensity = ui->groupBoxPowerSpectralDensity->isChecked(); // Спектрального разложения
    int saveStatus = 0;
        // Склейки
    saveStatus += divSignal.writeGluedParts(lastPath_);
        // Перемещения
    saveStatus += divSignal.writeDisplacement(lastPath_, "Перемещения.txt");
    saveStatus += divSignal.writeApproxDisplacement(lastPath_, "Аппрокс перемещения.txt");
        // Спектры
    if (isPowerSpectralDensity){
        WindowFunction windowFun = WindowFunction(ui->comboBoxWeightWindowType->currentIndex()); // Тип окна (HAMMING, HANN, BLACKMAN)
        double spectrumOverlapFactor = ui->spinBoxSpectrumOverlapFactor->value(); // Коэффициент перекрытия окон
        int lengthSpectrum =  ui->spinBoxSpectrumInterpolation->value(); // Число точек для интерполяции
        int windowSmoothWidth = ui->spinBoxSmoothWidth->value(); // Число точек для сглаживания
        divSignal.calculatePowerSpectralDensity(windowFun, spectrumOverlapFactor, lengthSpectrum, windowSmoothWidth); // Вычисление спектров
        saveStatus += divSignal.writeSpectrum(lastPath_); // Сохранение спектров
    }
        // Информация об уровнях
    saveStatus += divSignal.writeInfo(lastPath_, "Информация об уровнях.txt");
    if (saveStatus == 0) // В случае успешного сохранения
        emit this->accepted();
    this->hide(); // Скрытие окна
}

// Отображение уровней на графике
void LevelsWindow::showLevels(){
    static const double SHIFT_XMAX = 0.05; // Смещение максимума по оси абсцисс
    // Сигналы
    DataSignal const& accel = vecDataSignal_[ui->comboBoxAccel->currentIndex() - 1]; // Ускорения
    DataSignal displacement; // Перемещения
    // Параметры
    double levelStep = ui->spinBoxLevelStep->value();
    double overlapFactor = ui->spinBoxOverlapFactor->value();
    double smoothIntegrFactor = ui->spinBoxSmoothIntegrFactor->value();
    double smoothApproxFactor = ui->spinBoxSmoothApproxFactor->value();
    // Получение перемещения
    if (ui->comboBoxDisplacement->currentText().isEmpty())
        displacement = integrate(accel, 2, smoothIntegrFactor)[1];
    else
        displacement = vecDataSignal_[ui->comboBoxDisplacement->currentIndex() - 1];
    displacement.normalize(FIRST); // Перемещений
    DataSignal approxDisplacement = approximateSmoothSpline(displacement, smoothApproxFactor); // Аппроксимация перемещений
    // Подготовка контейнеров результирующих значений
    QVector<double> lowBoundLevels = {0}, upperBoundLevels = {0}; // Нижние и верхние границы уровней
    QVector<int> indLevels = {0}; // Индексы уровней
    int nLevels = 0; // Число уровней
    DivisionDataSignal::createLevels(approxDisplacement, calculationInd_, overlapFactor, levelStep,
                                     lowBoundLevels, upperBoundLevels, indLevels, nLevels); // Создание уровней
    if (nLevels == 0) return;
    clearAllPlot(); // Очистка всех графиков
    // Формирование вектора отсчетов
    int nSignal = calculationInd_.second + 1;
    QVector<double> XData(nSignal);
    for (int i = 0; i != nSignal; ++i)
        XData[i] = i + 1;
    // Построение сигналов
    QPen penPlot;
    penPlot.setStyle(Qt::SolidLine); // Стиль линии
    penPlot.setWidthF(1.0);
    // Перемещения
    penPlot.setColor(Qt::blue);
    plotGraph(XData, displacement.getData(calculationInd_.first, calculationInd_.second), penPlot);
    // Аппроксимированные перемещения
    penPlot.setColor(Qt::red);
    plotGraph(XData, approxDisplacement.getData(calculationInd_.first, calculationInd_.second), penPlot);
    // Построение уровней
    penPlot.setStyle(Qt::DashLine);
    QVector<double> XBound = {XData[0], XData[nSignal - 1]}; // Граничные значения
    double XPosLabel = (1.0 + SHIFT_XMAX / 2.0) * XBound[1];
    for (int i = 0; i != nLevels; ++i){
        QColor plotColor = i % 2 == 0 ? Qt::black : Qt::darkGreen;
        penPlot.setColor(plotColor);
        // Нижняя граница
        QVector<double> YLine = {lowBoundLevels[i], lowBoundLevels[i]};
        plotGraph(XBound, YLine, penPlot);
        // Верхняя граница
        YLine = {upperBoundLevels[i], upperBoundLevels[i]};
        plotGraph(XBound, YLine, penPlot);
        // Подпись уровня
        QCPItemText * label = new QCPItemText(ui->showLevelsPlot);
        double YPosLabel = (lowBoundLevels[i] + upperBoundLevels[i]) / 2.0;
        label->setText(QString::number(indLevels[i]));
        label->setColor(plotColor);
        label->position->setCoords(XPosLabel, YPosLabel);
    }
    ui->showLevelsPlot->xAxis->setRange(XBound[0], (1.0 + SHIFT_XMAX) * XBound[1]);
    ui->showLevelsPlot->replot(); // Обновление окна построения
}

// Построение графика
void LevelsWindow::plotGraph(QVector<double> const& X, QVector<double> const& Y, QPen penPlot, bool isReplot){
    ui->showLevelsPlot->addGraph(); // Добавление графика в конец
    ui->showLevelsPlot->graph()->setAdaptiveSampling(false); // Отключение сэмплирования отображаемых значений
    ui->showLevelsPlot->graph()->setPen(penPlot); // Выставление цвета графика
    ui->showLevelsPlot->graph()->setData(X, Y, true); // Передача отсортированных данных
    ui->showLevelsPlot->rescaleAxes(false); // Масштабирование осей
    if ( isReplot ) ui->showLevelsPlot->replot(); // Обновление окна построения
}

// Построение кривой
void LevelsWindow::plotCurve(QVector<double> const& X, QVector<double> const& Y, QPen penPlot, bool isReplot){
    int nPoint = X.size(); // Число точек в графике
    if (nPoint != Y.size()) return;
    // Формирование вектора порядка
    QVector<double> order(nPoint);
    for (int i = 0; i != nPoint; ++i)
        order[i] = i;
    // Создание кривой
    QCPCurve * someCurve = new QCPCurve(ui->showLevelsPlot->xAxis, ui->showLevelsPlot->yAxis);
    someCurve->setPen(penPlot); // Выставление цвета графика
    someCurve->setData(order, X, Y, true); // Передача отсортированных данных
    ui->showLevelsPlot->rescaleAxes(false); // Масштабирование осей
    if ( isReplot ) ui->showLevelsPlot->replot(); // Обновление окна построения
}

// Очистка графика
void LevelsWindow::clearAllPlot(){
    ui->showLevelsPlot->clearItems(); // Удаление надписей
    ui->showLevelsPlot->clearGraphs(); // Удаление графиков
    ui->showLevelsPlot->clearPlottables(); // Очистка кривых
    ui->showLevelsPlot->replot(); // Обновление окна построения
}

