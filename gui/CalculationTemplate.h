#ifndef CALCULATIONTEMPLATE_H
#define CALCULATIONTEMPLATE_H

#include <QHash>
#include <QVariant>

using WindowData = QHash<QString, QVariant>;

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
    void addWindowData(QString const& windowName, QString const& dataName, QVariant const& data); // Вставка данных поля окна
    void removeWindow(int index); // Удаление действия
    void clear(); // Очистка шаблона
    bool isEmpty() const { return windowsData_.isEmpty(); } // Проверка на пустоту
    // Управление записью действий
    void setStateRecord(bool state) { isRecord_ = state; } // Установить состояние
    bool isRecord() const { return isRecord_; } // Проверка состояния
    QList<QString> const& getSequenceOfWindows() const { return sequenceOfWindows_; }; // Получить последовательность действий
    // Чтение - запись
    int write(QString const& path, QString const& fileName) const; // Запись в бинарный файл
    int read(QString const& path, QString const& fileName) const; // Чтение бинарного файла
private:
    QHash<QString, WindowData> windowsData_; // Данные всех окон
    QList<QString> sequenceOfWindows_; // Последовательность действий
    bool isRecord_ = false; // Флаг записи шаблона
    QPair<int, int> estimationBoundaries_; // Границы расчета
    int widthWindow_; // Ширина окна
    int shiftWindow_; // Смещение окна
};

#endif // CALCULATIONTEMPLATE_H
