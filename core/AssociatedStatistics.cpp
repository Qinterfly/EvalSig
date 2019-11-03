#include <QtMath>
#include <thread>
#include <QDir>
#include <QTextStream>
#include <QTextCodec>
#include "AssociatedStatistics.h"

// ---- Неизменяемый класс относительных статистических характеристик ------------------------------------------

AssociatedStatistics::AssociatedStatistics(QVector<DataSignal> const& vecDataSignal, int widthWindow, int shiftMainWindow,
                                           int shiftCompareWindow, int indMainSignal) : vecDataSignal_(vecDataSignal),
    widthWindow_(widthWindow), shiftMainWindow_(shiftMainWindow), shiftCompareWindow_(shiftCompareWindow), indMainSignal_(indMainSignal)
{
    nSignal_ = vecDataSignal.size(); // Число сигналов
    if (nSignal_) nSize_ = nSignal_ - 1; // Определение размера матрицы
    // Выделение памяти
    vecNumberOfWindows_.resize(nSignal_); // Число окон по сигналам
    indCorrespond_.resize(nSize_);  // Вектор соответствия индекса в статистиках номеру сигнала
    // Предварительные оценки
    vecNumberOfWindows_ = calcNumberOfWindows(vecDataSignal_, widthWindow_, shiftMainWindow_, shiftCompareWindow_, indMainSignal_); // Числа окон по сигналам
}

// Установка параметров окон
void AssociatedStatistics::setWindowsParams(int widthWindow, int shiftMainWindow, int shiftCompareWindow){
    // Проверка необходимости изменения
    if (widthWindow_ == widthWindow && shiftMainWindow_ == shiftMainWindow && shiftCompareWindow_ == shiftCompareWindow)
        return;
    widthWindow_ = widthWindow; // Ширина окна
    shiftMainWindow_ = shiftMainWindow; // Смещение окна по главному сигналу
    shiftCompareWindow_ = shiftCompareWindow; // Смещение окна по сигналам для сравнения
    vecNumberOfWindows_ = calcNumberOfWindows(vecDataSignal_, widthWindow_, shiftMainWindow_, shiftCompareWindow_, indMainSignal_); // Оценка числа окон по сигналам
}

// Расчет статистик
int AssociatedStatistics::computeStatistics(){
    if (isEmpty()){ return 1; }
    allocateAllFields(); // Выделение памяти для всех полей
    findCorrespondence(); // Установление соответствия статистик и сигналов
    callMultiThread(&AssociatedStatistics::calcDistanceAmplitudeRegression); // Пересчет дистанций, амплитуд и регрессионных параметров
    callMultiThread(&AssociatedStatistics::calcSimilarity); // Расчет коэффициентов подобия
    return 0;
}

// Тело цикла пересчета для дистанций, амплитуд и регрессионных параметров
void AssociatedStatistics::calcDistanceAmplitudeRegression(int iStat){
    DataSignal const& mainSignal = vecDataSignal_[indMainSignal_]; // Ссылка на главный сигнал
    DataSignal const& compareSignal = vecDataSignal_[indCorrespond_[iStat]]; // Ссылка на сравниваемый сигнал
    int normSize = mainSignal.size(); // Размер для нормализации
    int nMainWindows = vecNumberOfWindows_[indMainSignal_]; // Число окон в главном сигнале
    int nCompareWindows = vecNumberOfWindows_[indCorrespond_[iStat]]; // Число окон в сравниваемом сигнале
    int lBoundMain = 0;    // Левая граница текущего окна главного сигнала
    int lBoundCompare = 0; // Левая граница текущего окна сравниваемого сигнала
    for (int i = 0; i != nMainWindows; ++i){ // Цикл по окнам главного сигнала
        lBoundCompare = 0;
        for (int j = 0; j != nCompareWindows; ++j){ // Цикл по окнам сравниваемого сигнала
            // Нахождение средних значений
            double meanX = 0, meanY = 0;
            for (int k = 0; k != widthWindow_; ++k){
                meanX += mainSignal[lBoundMain + k];
                meanY += compareSignal[lBoundCompare + k];
            }
            meanX /= widthWindow_; // Нормировка к реальной ширине окна
            meanY /= widthWindow_;
            // Нахождение параметров линейной регрессии
            double numeratorA = 0, denominatorA = 0; // Числитель и знаменатель углового коэффициента
            for (int k = 0; k != widthWindow_; ++k){
                numeratorA += (  mainSignal[lBoundMain + k] - meanX ) * ( compareSignal[lBoundCompare + k] - meanY );
                denominatorA += qPow( mainSignal[lBoundMain + k] - meanX, 2 );
            }
            if (numeratorA == 0.0 && denominatorA == 0.0)
                regressionParams_[iStat][i][j].first = 0; // Угловой коэффициент
            else
                regressionParams_[iStat][i][j].first = numeratorA / denominatorA; // Угловой коэффициент
            regressionParams_[iStat][i][j].second = meanY - regressionParams_[iStat][i][j].first * meanX; // Смещение прямой
            double alpha = qAtan(regressionParams_[iStat][i][j].first); // Угол наклона прямой
            double tSumDistance = 0; // Подсумма дистанции рассеяния
            double tXSignal = 0, tYSignal = 0; // Для амплитуды рассеяния
            for (int k = 0; k != widthWindow_; ++k){
                // Вычисление регрессионной функции
                double tLinearRegressionFun = regressionParams_[iStat][i][j].first * mainSignal[lBoundMain + k] + regressionParams_[iStat][i][j].second;
                // Вычисление подсуммы дистанции рассеяния
                tSumDistance += 1. / normSize * qFabs( compareSignal[lBoundCompare + k] - tLinearRegressionFun ) * qCos(alpha);
                // Для амплитуды рассеяния
                tXSignal += qAbs( mainSignal[lBoundMain + k] - meanX );
                tYSignal += qAbs( compareSignal[lBoundCompare + k] - meanY );
            }
            distanceScatter_[iStat][i][j] = tSumDistance; // Дистанция рассеяния
            if (tXSignal == 0.0 && tYSignal == 0.0)
                amplitudeScatter_[iStat][i][j] = 0;
            else
                amplitudeScatter_[iStat][i][j] = tSumDistance * normSize / qSqrt(qPow(tXSignal, 2) + qPow(tYSignal, 2)); // Амплитуда рассеяния
            lBoundCompare += shiftCompareWindow_; // Сдвиг границ окна для сравнения
        }
        lBoundMain += shiftMainWindow_; // Сдвиг границ главного окна
    }
}

// Тело цикла для расчета коэффициентов подобия
void AssociatedStatistics::calcSimilarity(int iStat){
    int nMainWindows = vecNumberOfWindows_[indMainSignal_]; // Число окон в главном сигнале
    int nCompareWindows = vecNumberOfWindows_[indCorrespond_[iStat]]; // Число окон в сравниваемом сигнале
    for (int i = 0; i != nMainWindows; ++i){ // Цикл по окнам главного сигнала
        for (int j = 0; j != nCompareWindows; ++j){ // Цикл по окнам сравниваемого сигнала
            similarityCoeffs_[iStat][i][j] = qSqrt(regressionParams_[iStat][i][j].first * regressionParams_[iStat][i][j].first);
            noiseCoeffs_[iStat][i][j] = qSqrt(amplitudeScatter_[iStat][i][j] * amplitudeScatter_[iStat][i][j]);
        }
    }
}

// Расчет числа окон по всем сигналам
QVector<int> AssociatedStatistics::calcNumberOfWindows(QVector<DataSignal> const& vecDataSignal, int widthWindow, int shiftMainWindow, int shiftCompareWindow, int indMainSignal){
    int shiftWindow = 0; // Параметры разбиения текущего сигнала
    int nSignal = vecDataSignal.size(); // Число сигналов
    QVector<int> vecNumberOfWindows(nSignal); // Вектор количества окон по сигналам
    for (int i = 0; i != nSignal; ++i){
        shiftWindow = shiftCompareWindow;
        if (i == indMainSignal) shiftWindow = shiftMainWindow; // Изменение параметров окна для главного сигнала
        int sizeSignal = vecDataSignal[i].size();
        int currWindow = 0; // Номер текущего окна
        // Пока текущая левая граница не достигнет конца правой расчетной границы
        for (int s = 0; s < sizeSignal; ){
            if (s + widthWindow > sizeSignal) // Проверка правой границы
                break; // Разрешены только цельные окна
            s += shiftWindow; // Сдвиг левой границы окна
            currWindow += 1; // Приращение счетчика окон
        }
        vecNumberOfWindows[i] = currWindow; // Установка числа окон (без учета среднего)
    }
    return vecNumberOfWindows;
}

// Выделение памяти для поля типа ArrayStatCharacters и ArrayRegressionParams
template<typename T>
void AssociatedStatistics::allocateField(T& field){
    if (isEmpty()) return; // Если сигналов меньше двух
    field.resize(nSize_); // Параметры линейной регрессии
    int nMainWindows = vecNumberOfWindows_[indMainSignal_]; // Число окон в главном сигнале
    int k = 0; // Счетчик сигналов
    for (int i = 0; i != nSignal_; ++i){
        if (i == indMainSignal_) continue; // Пропуск главного сигнала
        field[k].resize(nMainWindows); // Параметры линейной регрессии
        int nCurrentWindows = vecNumberOfWindows_[i]; // Число окон в текущем сигнале
        for (int j = 0; j != nMainWindows; ++j){
            field[k][j].resize(nCurrentWindows); // Параметры линейной регрессии
        }
        k = k + 1;
    }
}

// Выделение памяти для всех полей
void AssociatedStatistics::allocateAllFields(){
    allocateField(regressionParams_); // Параметры линейной регрессии
    allocateField(distanceScatter_);  // Дистанция рассеяния
    allocateField(similarityCoeffs_); // Коэффициенты подобия сигналов
    allocateField(amplitudeScatter_); // Амплитуда рассеяния
    allocateField(noiseCoeffs_);      // Коэффициенты шума
}

// Установление соответствия статистик и сигналов
void AssociatedStatistics::findCorrespondence(){
    int k = 0; // Счетчик сигналов
    for (int i = 0; i != nSignal_; ++i){
        if (i == indMainSignal_) continue; // Пропуск главного сигнала
        indCorrespond_[k] = i;
        k = k + 1;
    }
}

// Вызов метода в многопоточном режиме
void AssociatedStatistics::callMultiThread(void (AssociatedStatistics::*method)(int)){
    std::thread thread[nSize_]; // Создание потоков
    for (int i = 0; i != nSize_; ++i)
        thread[i] = std::thread(method, this, i);
    // Блокируем основной поток до выполнения вызванных
    for (int i = 0; i != nSize_; ++i)
        thread[i].join();
}

// Сохранение всех статистик
int AssociatedStatistics::writeAllStatistics(QString const& dirName) const{
    if (isEmpty()) return 1; // Проверка на пустоту статистик
    QDir dir(dirName); // Инициализация директории c добавлением разделителя
    if (!dir.exists()) // Проверка существования директории
        dir.mkpath(".");
    // Создание поддиректорий для каждой из статистик
    QVector<QString> vecStatName = {"Угловые коэффициенты", "Дистанции рассеяния",
                                "Коэффициенты подобия", "Амплитуды рассеяния", "Коэффициенты шума"}; // Имена статистик
    // Сохранение статистик по всем окнам
    int exitStatus = 0; // Код возврата
    exitStatus += writeStatistic(regressionParams_, dirName, vecStatName[0]); // Угловые коэффициенты
    exitStatus += writeStatistic(distanceScatter_, dirName, vecStatName[1]);  // Дистанция рассеяния
    exitStatus += writeStatistic(similarityCoeffs_, dirName, vecStatName[2]); // Коэффициенты подобия
    exitStatus += writeStatistic(amplitudeScatter_, dirName, vecStatName[3]); // Амплитуды рассеяния
    exitStatus += writeStatistic(noiseCoeffs_, dirName, vecStatName[4]); // Коэффициенты шума
    // Сохранение информации о расчете
    exitStatus += writeInfo(dirName, "Параметры расчета");
    return exitStatus;
}

// Сохранение информации о расчете
int AssociatedStatistics::writeInfo(QString const& dirName, QString const& fileName) const{
    QDir dir(dirName); // Инициализация директории c добавлением разделителя
    if (!dir.exists()) // Проверка существования директории
        dir.mkpath(".");
    QString fileFullPath = dirName + fileName; // Полный путь к файлу
    QFile file(fileFullPath); // Инициализация файла для записи
    if (!checkFile(fileFullPath, "write")){ return -1; } // Обработка ошибок
    file.open(QIODevice::WriteOnly | QIODevice::Text); // Открытие файла для записи
    QTextStream outputStream(&file); // Создание потока для записи
    outputStream.setCodec(QTextCodec::codecForLocale()); // Кодировка по системе
    // Запись информации о расчете
    outputStream << "Name of main signal: " << vecDataSignal_[indMainSignal_].getName() << endl;
    outputStream << "Width of window: " << widthWindow_ << endl;
    outputStream << "Shift of main window: " << shiftMainWindow_ << endl;
    outputStream << "Shift of comparing window: " << shiftCompareWindow_ << endl;
    file.close(); // Закрытие файла
    return 0;
}

// Сохранение выбранной статистики
// ArrayRegressionParams и ArrayStatCharacters
template<typename T>
int AssociatedStatistics::writeStatistic(T const& stat, QString const& dirName, QString const& statName) const{
    QString basePath = dirName + statName + QDir::separator(); // Базовая часть пути до статистики
    QVector<double> tData; // Контейнер статистик для выбранной пары сигналов
    int exitStatus = 0; // Код возврата
    int nMainWin = vecNumberOfWindows_[indMainSignal_];
    for (int iStat = 0; iStat != nSize_; ++iStat){ // По числу сигналов
        // Добавление статистики по всем окнам в контейнер
        PropertyDataSignal tProperty = vecDataSignal_[indCorrespond_[iStat]].getProperty(); // Получение свойств сигнала
        tProperty.physicalFactor_ = 1; // Безразмерные коэффициенты
        QString path = basePath + QFileInfo(tProperty.fileName_).baseName() + QDir::separator();
        QDir dir(path);
        if (!dir.exists()) dir.mkpath("."); // Создание директории
        for (int jMainWin = 0; jMainWin != nMainWin; ++jMainWin){
            tData = getWindowStatisticData(stat, iStat, jMainWin); // Получение временных данных
            tProperty.nCount_ = tData.size(); // Запись длины сигнала
            tProperty.measurePoint_ = statName; // Имя статистики
            DataSignal tDataSignal(tData, tProperty); // Создание временного сигнала
            QString fileName = QString::number(jMainWin + 1) + ".txt"; // Определение имени временного сигнала
            exitStatus += tDataSignal.writeDataFile(path, fileName); // Запись сигнала без конвертации
        }
    }
    return exitStatus;
}

// ---- Вспомогательные функции --------------------------------------------------------------------------------

// Получение оконного распределения статистики
    // ArrayRegressionParams
QVector<double> AssociatedStatistics::getWindowStatisticData(ArrayRegressionParams const& stat, int iStat, int jMainWin) const {
    int nWin = stat[iStat][jMainWin].size();
    QVector<double> tData(nWin); // Вектор данных для хранения статистик
    for (int s = 0; s != nWin; ++s){ // По всем окнам, за исключением среднего
        tData[s] = stat[iStat][jMainWin][s].first; // Угловые коэффициенты
    }
    return tData;
}
    // ArrayStatCharacters
QVector<double> AssociatedStatistics::getWindowStatisticData(ArrayStatCharacters const& stat, int iStat, int jMainWin) const {
    int nWin = stat[iStat][jMainWin].size();
    QVector<double> tData(nWin); // Вектор данных для хранения статистик
    for (int s = 0; s != nWin; ++s){ // По всем окнам, за исключением среднего
        tData[s] = stat[iStat][jMainWin][s]; // Угловые коэффициенты
    }
    return tData;
}

// -------------------------------------------------------------------------------------------------------------

