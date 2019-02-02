#include "mainwindow.h"
#include "ui_mainwindow.h"

// Конструктор главного окна
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    statSignal_(vecDataSignal_, widthTimeWindow_, overlapFactor_), // Статистики
    colorList_(QColor::colorNames()) // Список цветов
{
    ui->setupUi(this); // Инициализация графического интерфейса
    initializeCalculationParams(); // Выставление расчетных параметров
    clearSignalPropertyList(); // Очистка листа со свойствами сигнала
    initializeSignalPropertyList(); // Инициализация листа со свойствами сигнала
    initializeShowParams(); // Инициализация параметров для отображения

    // Создание соединений сигнал - слот
    connect(ui->actionAddSignal, SIGNAL(triggered()), this, SLOT(addSignal())); // Добавить сигнал
    connect(ui->actionRemoveSignal, SIGNAL(triggered()), this, SLOT(removeSignal())); // Удалить сигнал
    connect(ui->actionSaveSignal, SIGNAL(triggered()), this, SLOT(saveSignal())); // Удалить сигнал
    connect(ui->listFile, SIGNAL(itemSelectionChanged()), this, SLOT(setSignalProperty())); // Установка свойств сигнала
    connect(ui->actionNewProject, SIGNAL(triggered()), this, SLOT(clearProject())); // Новый проект
    connect(ui->tableFileProperty, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(setColor(int, int))); // Установка цвета сигнала
}

// Деструктор главного окна
MainWindow::~MainWindow()
{
    delete ui;
}

// ----  Чтение, запись и удаление сигналов ------------------------------------------------------------------------

    // Добавить сигнал
void MainWindow::addSignal(){
    // Организация диалога с пользователем
    QStringList listFullFilePath = QFileDialog::getOpenFileNames(this, "Выберите один или несколько файлов с временными сигналами", lastPath_, "Text files (*.txt)");
    if (listFullFilePath.isEmpty()) return;
    int indFile = 0; // Номер файла в обработке
    for ( QString fullFilePath : listFullFilePath){
        int exitStatus = 0; // Код возврата
        ++indFile; // Инкремент номера обрабатываемого файла
        QFileInfo infoName(fullFilePath); // Создание информационного объекта
        QString fileName = infoName.fileName(); // Имя файла
        if (indFile == 1) // Выставление релевантного пути по первому файлу
            lastPath_ = infoName.absolutePath() + QDir::separator(); // Путь к файлу ( + запись в последний выбранный)
        // Добавление сигнала в контейнеры
        exitStatus = statSignal_.addSignal(DataSignal(lastPath_, fileName)); // Расчет статистик + пополнение вектора сигналов
        if (exitStatus == 0){ // При успешном добавлении в контейнер
            QListWidgetItem * item = new QListWidgetItem(infoName.baseName()); // Занесение в список имени без расширения
            int randColorInd = QRandomGenerator::global()->bounded(colorList_.size()); // Случайный цвет из контейнера цветов
            item->setData(Qt::DecorationRole, QColor(colorList_[randColorInd])); // Задание цвета для сигнала
            ui->listFile->insertItem(ui->listFile->count(), item); // Запись элемента в список
            if (showWindow_ > statSignal_.getNumberOfWindows()) // Проверка номера отображаемого окна
                showWindow_ = 0; // Сброс в случае превышение реального количества окон
            statusBar()->showMessage("Сигнал " + fileName + " успешно прочитан"); // Вывод сообщения в statusBar
        }
    }
    // Установка свойств по последнему сигналу
    int nItem = ui->listFile->count() - 1; // Число элементов в списке
    ui->listFile->setCurrentRow(nItem); // Фокус на последнем элементе (вызов setSignalProperty() по сигналу)
}

    // Удалить сигнал
void MainWindow::removeSignal(){
    int deleteInd = ui->listFile->currentRow(); // Индекс элемента для удаления
    if (ui->listFile->count() == 0 || deleteInd == -1) // При отсутствии элементов в списке
        return;
    if (statSignal_.removeSignal(deleteInd) == 0){ // Если удаление прошло успешно
        statusBar()->showMessage("Сигнал " + ui->listFile->item(deleteInd)->text() + " успешно удален"); // Вывод сообщения в statusBar
        ui->listFile->takeItem(deleteInd); // Удаляем элемент из списка
    }
}

    // Сохранить сигнал
void MainWindow::saveSignal(){
    int saveInd = ui->listFile->currentRow(); // Индекс файла для сохранения
    if (saveInd == -1) return; // Проверка на пустоту списка
    QString fullFilePath = QFileDialog::getSaveFileName(this, "", lastPath_, "Text files (*.txt)"); // Диалог с пользователем
    if (fullFilePath.isEmpty()) return; // Проверка корректности выбора
    if (!fullFilePath.endsWith(".txt"))
        fullFilePath += ".txt"; // Добавление расширения, если это необходимо
    QFileInfo infoName(fullFilePath); // Инфо для деления имен
    QString fileName = infoName.fileName(); // Имя файла
    lastPath_ = infoName.absolutePath() + QDir::separator(); // Путь к файлу ( + запись в последний выбранный)
    int exitStatus = vecDataSignal_[saveInd].writeDataFile(lastPath_, fileName); // Запись сигнала в файл
    if (exitStatus == 0)
        statusBar()->showMessage("Сигнал " + fileName + " успешно записан"); // Вывод сообщения в statusBar
}

// ---- Инициализация параметров ---------------------------------------------------------------------------------

// Инициализация листа со свойствами сигнала
void MainWindow::initializeSignalPropertyList(){
    // Выставление параметров доступа
        // Отключение изменений столбца свойств
    int nTable = ui->tableFileProperty->rowCount(); // Число строк
    for (int i = 0; i != nTable; ++i){
        QTableWidgetItem * item = ui->tableFileProperty->item(i, 0); // Получение элемента таблицы
        item->setFlags(item->flags() ^ Qt::ItemIsEditable); // Отключение возможности изменения содержимого колонок
    }
    // Отключение изменений специальных строк
        // Полное имя файла
    QTableWidgetItem * item = ui->tableFileProperty->item(0, 1);
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
        // Количество отсчётов
    item = ui->tableFileProperty->item(11, 1);
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
        // Цвет графика (меняется методом при клике)
    item = ui->tableFileProperty->item(12, 1);
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
}

// Выставление расчетных параметров
void MainWindow::initializeCalculationParams(){
    // Запись параметров временного окна
    widthTimeWindow_ = ui->spinBoxTimeWidth->value(); // Ширина окна
    overlapFactor_ = ui->spinBoxOverlapFactor->value(); // Коэффициент перекрытия
    statSignal_.setWindowProperty(widthTimeWindow_, overlapFactor_); // Установка параметров окна
}

// Инициализация параметров для отображения
void MainWindow::initializeShowParams(){
    showWindow_ = ui->spinBoxShowWindow->value(); // Номер окна для показа
}

// ---- Set методы --------------------------------------------------------------------------------------------

// Установка свойств сигнала
void MainWindow::setSignalProperty(){
    int signalInd = ui->listFile->currentRow(); // Индекс сигнала в фокусе
    if (signalInd == -1){ // Проверка на пустоту списка исходных сигналов
        clearSignalPropertyList(); // Очистка листа с свойствами сигнала
        return;
    }
    PropertyDataSignal const& propSignal = vecDataSignal_[signalInd].getProperty(); // Получение свойств по индексу
    ui->tableFileProperty->item(0, 1)->setText(propSignal.path_ + propSignal.fileName_); // Полный путь к файлу
    ui->tableFileProperty->item(1, 1)->setText(propSignal.dateAndTime_); // Дата и время
    ui->tableFileProperty->item(2, 1)->setText(propSignal.measureObject_); // Объект измерения
    ui->tableFileProperty->item(3, 1)->setText(propSignal.measurePoint_); // Точка установки
    ui->tableFileProperty->item(4, 1)->setText(propSignal.currentCount_); // Текущие отсчеты
    ui->tableFileProperty->item(5, 1)->setText(QString::number(propSignal.temperature_)); // Температура
    ui->tableFileProperty->item(6, 1)->setText(propSignal.sensorType_); // Тип датчика
    ui->tableFileProperty->item(7, 1)->setText(QString::number(propSignal.physicalFactor_)); // Физический коэффициент
    ui->tableFileProperty->item(8, 1)->setText(propSignal.measureUnit_); // Единица измерения
    ui->tableFileProperty->item(9, 1)->setText(QString::number(propSignal.scanPeriod_)); // Период опроса датчика
    ui->tableFileProperty->item(10, 1)->setText(propSignal.characterisic_); // Характеристика
    ui->tableFileProperty->item(11, 1)->setText(QString::number(propSignal.nCount_)); // Количество отсчетов
    // Цвет графика
    QListWidgetItem * listItem = ui->listFile->item(signalInd); // Получение элемента списка
    QVariant varItem = listItem->data(Qt::DecorationRole); // Получение цвета в формате QVariant
    ui->tableFileProperty->item(12, 1)->setText(varItem.value<QColor>().name()); // Запись QVariant -> QColor
    ui->tableFileProperty->item(12, 1)->setData(Qt::DecorationRole, listItem->data(Qt::DecorationRole));
}

// Установка цвета сигнала
void MainWindow::setColor(int row, int column){
    if (ui->listFile->currentRow() == -1) return; // Проверка на наличие выбранного элемента
    if (row != 12 || column != 1) return; // Проверка выбора ячейки с цветом
    QColor selectedColor = QColorDialog::getColor(Qt::white, this, "Выберите цвет сигнала");
    if (!selectedColor.isValid()) return; // Проверка выбора корректного цвета
    ui->listFile->currentItem()->setData(Qt::DecorationRole, selectedColor); // Установка цвета в списке сигналов
    ui->tableFileProperty->item(row, column)->setData(Qt::DecorationRole, selectedColor); // Установка цвета в свойствах сигнала
}

// ---- Методы очистки ----------------------------------------------------------------------------------------

// Очистка листа со свойствами сигнала
void MainWindow::clearSignalPropertyList(){
    int nTable = ui->tableFileProperty->rowCount(); // Число строк
    for (int i = 0; i != nTable; ++i){
        QTableWidgetItem * item = ui->tableFileProperty->item(i, 1); // Получение элемента таблицы
        item->setText(""); // Очистка элементов таблицы
        if (i == nTable - 1)
            item->setData(Qt::DecorationRole, 0); // Очистка поля выбора цвета
    }
}

// Очистка проекта
void MainWindow::clearProject(){
while(ui->listFile->count() != 0)
    removeSignal(); // Удаление всех сигналов
}
