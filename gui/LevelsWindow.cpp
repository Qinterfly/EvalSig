#include "LevelsWindow.h"
#include "ui_LevelsWindow.h"
#include <QFileDialog>
#include "core/NumericalFunctions.h"
#include "core/DivisionDataSignal.h"

// ---- Разбиение на уровни ------------------------------------------------------------------------------------

static QString const& WINDOW_NAME = "LevelsWindow";

// Конструктор
LevelsWindow::LevelsWindow(CalculationTemplate & calcTemplate, QVector<DataSignal> const& vecDataSignal, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LevelsWindow),
    calcTemplate_(calcTemplate),
    vecDataSignal_(vecDataSignal)
{
    ui->setupUi(this);
    // Создание соединений сигнал - слот
    connect(ui->comboBoxBase, SIGNAL(currentIndexChanged(int)), this, SLOT(setSaveState(int))); // Установка возможности сохранения
    connect(ui->pushButtonSaveLevels, SIGNAL(clicked()), this, SLOT(save())); // Сохранение результатов
    connect(ui->pushButtonShowLevels, SIGNAL(clicked()), this, SLOT(showLevels())); // Отображение уровней на графике
    connect(ui->pushButtonAssessNumberOfLevels, SIGNAL(clicked()), this, SLOT(assessNumberOfLevels())); // Оценить число уровней
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
    ui->comboBoxBase->clear();
    ui->comboBoxSupport->clear();
    // Добавление пустых элементов
    ui->comboBoxBase->addItem("");
    ui->comboBoxSupport->addItem("");
    // Добавление имен сигналов в ускорения и перемещения
    for (int i = 0; i != nItem; ++i){
        QString const& tName = listSignals.item(i)->text();
        ui->comboBoxBase->addItem(tName);
        ui->comboBoxSupport->addItem(tName);
    }
    // Выбор пустых элементов
    ui->comboBoxBase->setCurrentIndex(0);
    ui->comboBoxSupport->setCurrentIndex(0);
}

// Установка расчетных границ
void LevelsWindow::setEstimationBoundaries(QPair<int, int> const& estimationBoundaries){
    estimationBoundaries_ = estimationBoundaries;
    calculationInd_ = {estimationBoundaries_.first - 1, estimationBoundaries_.second - 1}; // Расчетные индексы
}

// Установка пути по умолчанию
void LevelsWindow::setLastPath(QString const& lastPath){
    lastPath_ = lastPath;
}

// Применить расчетный шаблон
void LevelsWindow::applyCalculationTemplate(int indBaseSignal, int indSupportSignal){
    if ( !calcTemplate_.contains(WINDOW_NAME) ) return;
    ui->comboBoxBase->setCurrentIndex(indBaseSignal); // Базовый сигнал
    ui->comboBoxSupport->setCurrentIndex(indSupportSignal); // Опорный сигнал
    WindowData const& windowData = *calcTemplate_.getWindowData(WINDOW_NAME); // Получение новых данных окна
    // Параметры разбиения
    ui->spinBoxLevelStep->setValue(windowData["levelStep"].toDouble()); // Смещение уровней
    ui->spinBoxOverlapFactor->setValue(windowData["overlapFactor"].toDouble()); // Перекрытие уровней
    ui->spinBoxSmoothIntegrFactor->setValue(windowData["smoothIntegrFactor"].toDouble()); // Коррекция интегралов
    ui->spinBoxSmoothApproxFactor->setValue(windowData["smoothApproxFactor"].toDouble()); // Сглаживание перемещений
    ui->spinBoxTruncatePercent->setValue(windowData["truncatePercent"].toDouble()); // Усечение фрагментов
    ui->spinBoxDepthGluing->setValue(windowData["depthGluing"].toDouble()); // Глубина склейки
    // Данные спектрального разложения
    ui->groupBoxPowerSpectralDensity->setChecked(windowData["isPowerSpectralDensity"].toBool()); // Флаг спектрального разложения
    ui->comboBoxWeightWindowType->setCurrentIndex(windowData["windowFun"].toInt()); // Тип окна (HAMMING, HANN, BLACKMAN)
    ui->spinBoxSpectrumOverlapFactor->setValue(windowData["spectrumOverlapFactor"].toDouble()); // Коэффициент перекрытия окон
    ui->spinBoxSpectrumInterpolation->setValue(windowData["lengthSpectrum"].toInt()); // Число точек для интерполяции
    ui->spinBoxSmoothWidth->setValue(windowData["windowSmoothWidth"].toInt()); // Число точек для сглаживания
}

// Установка возможности сохранения
void LevelsWindow::setSaveState(int){
    if (!ui->comboBoxBase->currentText().isEmpty()){
        ui->pushButtonSaveLevels->setEnabled(true);
        ui->pushButtonShowLevels->setEnabled(true);
        ui->pushButtonAssessNumberOfLevels->setEnabled(true);
    }
    else {
        ui->pushButtonSaveLevels->setEnabled(false);
        ui->pushButtonShowLevels->setEnabled(false);
        ui->pushButtonAssessNumberOfLevels->setEnabled(false);
        ui->groupBoxParams->setTitle("Параметры разбиения");
        clearAllPlot();
    }
}

// Сохранение и расчет
void LevelsWindow::save(bool isUserCalc){
    // Диалог с пользователем для выбора директории для сохранения
    if (isUserCalc){
        QString saveDir = QFileDialog::getExistingDirectory(this, "", lastPath_, QFileDialog::ShowDirsOnly); // Диалоговое окно
        // Проверка корректности выбора
        if (saveDir.isEmpty()) return;
        lastPath_ = saveDir + QDir::separator(); // Запись последней директории
    }
    // Сигналы
    DataSignal base = vecDataSignal_[ui->comboBoxBase->currentIndex() - 1]; // Базовый
    DataSignal support; // Опорный
    // Параметры
    double levelStep = ui->spinBoxLevelStep->value();
    double overlapFactor = ui->spinBoxOverlapFactor->value();
    double smoothIntegrFactor = ui->spinBoxSmoothIntegrFactor->value();
    double smoothApproxFactor = ui->spinBoxSmoothApproxFactor->value();
    double truncatePercent = ui->spinBoxTruncatePercent->value();
    double depthGluing = ui->spinBoxDepthGluing->value();
    // Получение перемещения
    if (ui->comboBoxSupport->currentText().isEmpty())
        support = integrate(base, 2, smoothIntegrFactor)[1];
    else
        support = vecDataSignal_[ui->comboBoxSupport->currentIndex() - 1];
    // Нормализация данных
    base.normalize(FIRST); // Базового
    support.normalize(FIRST); // Опорного
    DataSignal approxSupport = approximateSmoothSpline(support, smoothApproxFactor); // Аппроксимация опорного
    // Вызов расчетных методов
    DivisionDataSignal divSignal(base, support, approxSupport, levelStep, overlapFactor, truncatePercent, depthGluing,
                                    estimationBoundaries_.first, estimationBoundaries_.second);
    divSignal.calculateLevels(); // Расчет уровней
    // Сохранение результатов
    bool isPowerSpectralDensity = ui->groupBoxPowerSpectralDensity->isChecked(); // Спектрального разложения
    int saveStatus = 0;
        // Склейки
    saveStatus += divSignal.writeGluedParts(lastPath_);
        // Опорный
    saveStatus += divSignal.writeSupport(lastPath_, "Опорный.txt");
    saveStatus += divSignal.writeApproxSupport(lastPath_, "Аппроксимация опорного.txt");
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
    if (saveStatus == 0 && isUserCalc) // В случае успешного сохранения
        emit this->accepted();
    this->hide(); // Скрытие окна
    // Заполнение расчетного шаблона
    if ( saveStatus != 0 || !calcTemplate_.isRecord() || !isUserCalc ) return;
    // Параметры разбиения
    calcTemplate_.addWindowData(WINDOW_NAME, "levelStep", levelStep); // Смещение уровней
    calcTemplate_.addWindowData(WINDOW_NAME, "overlapFactor", overlapFactor); // Перекрытие уровней
    calcTemplate_.addWindowData(WINDOW_NAME, "smoothIntegrFactor", smoothIntegrFactor); // Коррекция интегралов
    calcTemplate_.addWindowData(WINDOW_NAME, "smoothApproxFactor", smoothApproxFactor); // Сглаживание перемещений
    calcTemplate_.addWindowData(WINDOW_NAME, "truncatePercent", truncatePercent); // Усечение фрагментов
    calcTemplate_.addWindowData(WINDOW_NAME, "depthGluing", depthGluing); // Глубина склейки
    // Данные спектрального разложения
    calcTemplate_.addWindowData(WINDOW_NAME, "isPowerSpectralDensity", isPowerSpectralDensity); // Флаг спектрального разложения
    calcTemplate_.addWindowData(WINDOW_NAME, "windowFun", ui->comboBoxWeightWindowType->currentIndex()); // Тип окна (HAMMING, HANN, BLACKMAN)
    calcTemplate_.addWindowData(WINDOW_NAME, "spectrumOverlapFactor", ui->spinBoxSpectrumOverlapFactor->value()); // Коэффициент перекрытия окон
    calcTemplate_.addWindowData(WINDOW_NAME, "lengthSpectrum", ui->spinBoxSpectrumInterpolation->value()); // Число точек для интерполяции
    calcTemplate_.addWindowData(WINDOW_NAME, "windowSmoothWidth", ui->spinBoxSmoothWidth->value()); // Число точек для сглаживания
}

// Отображение уровней на графике
void LevelsWindow::showLevels(){
    static const double SHIFT_XMAX = 0.05; // Смещение максимума по оси абсцисс
    // Сигналы
    DataSignal const& base = vecDataSignal_[ui->comboBoxBase->currentIndex() - 1]; // Базовый
    DataSignal support; // Опорный
    // Параметры
    double levelStep = ui->spinBoxLevelStep->value();
    double overlapFactor = ui->spinBoxOverlapFactor->value();
    double smoothIntegrFactor = ui->spinBoxSmoothIntegrFactor->value();
    double smoothApproxFactor = ui->spinBoxSmoothApproxFactor->value();
    // Получение опорного
    if (ui->comboBoxSupport->currentText().isEmpty())
        support = integrate(base, 2, smoothIntegrFactor)[1];
    else
        support = vecDataSignal_[ui->comboBoxSupport->currentIndex() - 1];
    support.normalize(FIRST); // Опорного
    DataSignal approxSupport = approximateSmoothSpline(support, smoothApproxFactor); // Аппроксимация опорного
    // Подготовка контейнеров результирующих значений
    QVector<double> lowBoundLevels = {0}, upperBoundLevels = {0}; // Нижние и верхние границы уровней
    QVector<int> indLevels = {0}; // Индексы уровней
    int nLevels = 0; // Число уровней
    DivisionDataSignal::createLevels(approxSupport, calculationInd_, overlapFactor, levelStep,
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
    // Опорный
    penPlot.setColor(Qt::blue);
    plotGraph(XData, support.getData(calculationInd_.first, calculationInd_.second), penPlot);
    // Аппроксимированный опорный
    penPlot.setColor(Qt::red);
    plotGraph(XData, approxSupport.getData(calculationInd_.first, calculationInd_.second), penPlot);
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

// Оценить число уровней
void LevelsWindow::assessNumberOfLevels(){
    DataSignal const& base = vecDataSignal_[ui->comboBoxBase->currentIndex() - 1]; // Базовый
    DataSignal support; // Опорный
    // Параметры
    double levelStep = ui->spinBoxLevelStep->value();
    double overlapFactor = ui->spinBoxOverlapFactor->value();
    double smoothIntegrFactor = ui->spinBoxSmoothIntegrFactor->value();
    double smoothApproxFactor = ui->spinBoxSmoothApproxFactor->value();
    // Получение опорного
    if (ui->comboBoxSupport->currentText().isEmpty())
        support = integrate(base, 2, smoothIntegrFactor)[1];
    else
        support = vecDataSignal_[ui->comboBoxSupport->currentIndex() - 1];
    support.normalize(FIRST); // Опорного
    DataSignal approxSupport = approximateSmoothSpline(support, smoothApproxFactor); // Аппроксимация опорного
    int nLevels = DivisionDataSignal::assessNumberOfLevels(approxSupport, calculationInd_, overlapFactor, levelStep); // Оценка числа уровней
    ui->groupBoxParams->setTitle("Параметры разбиения [ n = " + QString::number(nLevels) + " ]");
}

// Построение графика
void LevelsWindow::plotGraph(QVector<double> const& X, QVector<double> const& Y, QPen const& penPlot, bool isReplot){
    ui->showLevelsPlot->addGraph(); // Добавление графика в конец
    ui->showLevelsPlot->graph()->setAdaptiveSampling(false); // Отключение сэмплирования отображаемых значений
    ui->showLevelsPlot->graph()->setPen(penPlot); // Выставление цвета графика
    ui->showLevelsPlot->graph()->setData(X, Y, true); // Передача отсортированных данных
    ui->showLevelsPlot->rescaleAxes(false); // Масштабирование осей
    if ( isReplot ) ui->showLevelsPlot->replot(); // Обновление окна построения
}

// Построение кривой
void LevelsWindow::plotCurve(QVector<double> const& X, QVector<double> const& Y, QPen const& penPlot, bool isReplot){
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

// -------------------------------------------------------------------------------------------------------------
