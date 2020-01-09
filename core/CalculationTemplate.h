#ifndef CALCULATIONTEMPLATE_H
#define CALCULATIONTEMPLATE_H

#include <QHash>
#include <QVariant>

using WindowData = QHash<QString, QVariant>;
static int const DEFAULT_VERSION = 1;

// Расчетный шаблон для обработки файлов
class CalculationTemplate
{
public:
    CalculationTemplate() = default; // Конструктор
    ~CalculationTemplate() = default; // Деструктор
    CalculationTemplate(CalculationTemplate const&) = delete; // Конструктор копирования
    CalculationTemplate operator=(CalculationTemplate const&) = delete; // Оператор присваивания
    // Пользовательские методы
    void setStatParams(QPair <int, int> const& estimationBoundaries, int widthWindow, int shiftWindow); // Установка параметров статистик
    void setNote(QString const& note) { note_ = note; }; // Установка примечания
    void addWindowData(QString const& windowName, QString const& dataName, QVariant const& data); // Вставка данных поля окна
    void removeWindow(int index); // Удаление действия
    void clear(); // Очистка шаблона
    // Проверка размерности
    bool isEmpty() const { return windowsData_.isEmpty(); } // Проверка на пустоту
    int lengthSequence() const { return sequenceOfWindows_.size(); } // Длина последовательности
    // Обращение к полям
    int version() const { return version_; } // Версия шаблона
    QString const& date() const { return date_; } // Дата
    QString const& note() const { return note_; } // Примечание
    int widthWindow() const { return widthWindow_; } // Ширина окна
    int shiftWindow() const { return shiftWindow_; } // Смещение окна
    QPair<int, int> const& estimationBoundaries() const { return estimationBoundaries_; } // Границы расчета
    bool contains(QString const& windowName) const { return windowsData_.contains(windowName); } // Проверка наличия данных окна
    QHash<QString, WindowData>::const_iterator getWindowData(QString const& windowName) const { return windowsData_.constFind(windowName); } // Получение данных окна
    // Управление записью действий
    void setStateRecord(bool state) { isRecord_ = state; } // Установить состояние
    bool isRecord() const { return isRecord_; } // Проверка состояния
    QList<QString> const& getSequenceOfWindows() const { return sequenceOfWindows_; }; // Получить последовательность действий
    // Чтение - запись
    int write(QString const& path, QString const& fileName); // Запись в бинарный файл
    int read(QString const& path, QString const& fileName); // Чтение бинарного файла
private:
    QHash<QString, WindowData> windowsData_; // Данные всех окон
    QList<QString> sequenceOfWindows_; // Последовательность действий
    bool isRecord_ = false; // Флаг записи шаблона
    QPair<int, int> estimationBoundaries_; // Границы расчета
    int widthWindow_; // Ширина окна
    int shiftWindow_; // Смещение окна
    int version_ = DEFAULT_VERSION; // Версия
    QString date_ = ""; // Дата
    QString note_ = ""; // Примечание к шаблону
};

#endif // CALCULATIONTEMPLATE_H
