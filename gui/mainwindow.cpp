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
    initializeSystemParams(); // Установка параметров используемой операционной системы
    clearSignalPropertyList(); // Очистка листа со свойствами сигнала
    initializeSignalPropertyList(); // Инициализация листа со свойствами сигнала
    initializeShowParams(); // Инициализация параметров для отображения

    // Создание соединений сигнал - слот
    connect(ui->actionAddSignal, SIGNAL(triggered()), this, SLOT(addSignal())); // Добавить сигнал
    connect(ui->actionRemoveSignal, SIGNAL(triggered()), this, SLOT(removeSignal())); // Удалить сигнал

}

// Деструктор главного окна
MainWindow::~MainWindow()
{
    delete ui;
}

// ---- Чтение / запись сигналов ------------------------------------------------------------------------

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
        if (indFile == 1){ // Выставление релевантного пути по первому файлу
            lastPath_ = infoName.absolutePath(); // Путь к файлу ( + запись в последний выбранный)
            lastPath_ += pathSymbol_; // Добавление символа разделения директорий
        }
        // Добавление сигнала в контейнеры
        exitStatus = statSignal_.addSignal(DataSignal(lastPath_, fileName)); // Расчет статистик + пополнение вектора сигналов
        if (exitStatus == 0){ // При успешном добавлении в контейнер
            QListWidgetItem * item = new QListWidgetItem(infoName.baseName()); // Занесение в список имени без расширения
            int randColorInd = QRandomGenerator::global()->bounded(colorList_.size()); // Случайный цвет из контейнера цветов
            item->setData(Qt::DecorationRole, QColor(colorList_[randColorInd])); // Задание цвета для сигнала
            ui->listFile->insertItem(ui->listFile->count(), item); // Запись элемента в список
            if (showWindow_ > statSignal_.getNumberOfWindows()) // Проверка номера отображаемого окна
                showWindow_ = 0; // Сброс в случае превышение реального количества окон
            // TODO -> отображение свойств по индексу сигнала
        }
    }
}

    // Удалить сигнал
void MainWindow::removeSignal(){

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
    item = ui->tableFileProperty->item(12, 1);
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
        // Цвет графика (меняется методом при клике)
    item = ui->tableFileProperty->item(13, 1);
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
}

// Установка параметров используемой операционной системы
void MainWindow::initializeSystemParams(){
    // Выставление символа разделения директорий в пути
    #ifdef Q_OS_UNIX
        pathSymbol_ = "/"; // Для unix
    #endif
    #ifdef Q_OS_WIN
        pathSymbol_ = "\"; // Для Winidows
    #endif
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
