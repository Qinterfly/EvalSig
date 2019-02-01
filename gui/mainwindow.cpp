#include "mainwindow.h"
#include "ui_mainwindow.h"

// Конструктор главного окна
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    statSignal_(vecDataSignal_, widthTimeWindow_, overlapFactor_)
{
    ui->setupUi(this); // Инициализация графического интерфейса
    setSystemParams(); // Установка параметров используемой операционной системы
    clearSignalPropertyList(); // Очистка листа со свойствами сигнала
    initializeSignalPropertyList(); // Инициализация листа со свойствами сигнала

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
    int exitValue = -1; // Код возврата
    // Организация диалога с пользователем
    QStringList listFullFilePath = QFileDialog::getOpenFileNames(this, "Выберите один или несколько файлов с временными сигналами", lastPath_, "Text files (*.txt)");
    if (listFullFilePath.isEmpty()) return;
    int iPos = 0; // Номер файла в обработке
    for ( QString fullFilePath : listFullFilePath){
        ++iPos;
        QFileInfo infoName(fullFilePath); // Создание информационного объекта
        QString fileName = infoName.fileName(); // Имя файла
        if (iPos == 1){ // Выставление релевантного пути по первому файлу
            lastPath_ = infoName.absolutePath(); // Путь к файлу ( + запись в последний выбранный)
            lastPath_ += pathSymbol_; // Добавление символа разделения директорий
        }
        // Добавление сигнала в контейнеры
        // TODO -> ЧТЕНИЕ С ГРАФИЧЕСКОГО ОКНА
                widthTimeWindow_ = 10; overlapFactor_ = 0.5; // Debug only
        // --
        if (statSignal_.isEmpty()) // Если список пуст, то устанавливаем параметры окна
            statSignal_.setWindowProperty(widthTimeWindow_, overlapFactor_);

        // TODO -> ТУТ ДОЛЖНА БЫТЬ ПРОВЕРКА НА WIDTH_TIME_WINDOW_ > MIN_SIZE_SIGNALS

        exitValue = statSignal_.addSignal(DataSignal(lastPath_, fileName)); // Расчет статистик + пополнение вектора сигналов
        if (exitValue == 0){ // При успешном добавлении в контейнер
            QListWidgetItem * item = new QListWidgetItem(infoName.baseName());

            // TODO -> ВЫБОР ЦВЕТА ИЗ СПИСКА

            item->setData(Qt::DecorationRole, QColor(255, 40, 0, 255));
            ui->listFile->insertItem(ui->listFile->count(), item);
        }
    }
}

    // Удалить сигнал
void MainWindow::removeSignal(){

}

// ---- Свойства сигнала ---------------------------------------------------------------------------------

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

// Очистка листа со свойствами сигнала
void MainWindow::clearSignalPropertyList(){
    int nTable = ui->tableFileProperty->rowCount(); // Число строк
    for (int i = 0; i != nTable; ++i){
        QTableWidgetItem * item = ui->tableFileProperty->item(i, 1); // Получение элемента таблицы
        item->setText(""); // Очистка элементов таблицы
        if (i == nTable - 1) item->setData(Qt::DecorationRole, 0); // Очистка поля выбора цвета
    }
}

// ---- Вспомогательные ---------------------------------------------------------------------------------------

// Установка параметров используемой операционной системы
void MainWindow::setSystemParams(){
    // Выставление символа разделения директорий в пути
    #ifdef Q_OS_UNIX
        pathSymbol_ = "/"; // Для unix
    #endif
    #ifdef Q_OS_WIN
        pathSymbol_ = "\"; // Для Winidows
    #endif
}

