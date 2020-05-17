#ifndef QCUSTOMPLOTZOOM_H
#define QCUSTOMPLOTZOOM_H

#include <QPoint>
#include "include/QCustomPlot.h"

class QRubberBand;
class QMouseEvent;
class QWidget;

struct QCustomPlotZoom : public QCustomPlot {
    Q_OBJECT
public:
    QCustomPlotZoom(QWidget * parent = nullptr);
    ~QCustomPlotZoom() override;
    void setZoomEnabled(bool enabled); // Переключение возможности масштабирования
    void setKeyAxes(QPair<int, int> indexes); // Установка осей для масштабирования
private slots:
    void mousePressEvent(QMouseEvent * event) override; // При нажатии кнопки мыши
    void mouseMoveEvent(QMouseEvent * event) override; // При сдвиге мыши
    void mouseReleaseEvent(QMouseEvent * event) override; // При отжатии кнопки мыши
    void enterEvent(QEvent * event) override; // При входе в область виджета
    void leaveEvent(QEvent * event) override; // При выходе из области виджета
    void showCoordTag(QMouseEvent * event); // Отображение курсорной подсказки
private:
    QRubberBand * rubberBand_;
    QPoint origin_; // Положение левого верхнего угла прямоугольника
    bool isZoomed_ = false; // Флаг масштабированного изображения
    QCPRange rangeXAxis_; // Диапазон по дополнительной горизонтальной оси
    QCPRange rangeYAxis_; // Диапазон по дополнительной вертикальной оси
    bool isZoomEnabled_ = true; // Флаг доступности масштабирования
    // Указатели на главные оси
    QCPAxis * keyXAxis_; // Абсцис
    QCPAxis * keyYAxis_; // Ординат
};

#endif // QCUSTOMPLOTZOOM_H
