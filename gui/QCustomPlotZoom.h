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
    QCustomPlotZoom(QWidget * parent = 0);
    virtual ~QCustomPlotZoom();
private slots:
    void mousePressEvent(QMouseEvent * event) override; // При нажатии кнопки мыши
    void mouseMoveEvent(QMouseEvent * event) override; // При сдвиге мыши
    void mouseReleaseEvent(QMouseEvent * event) override; // При отжатии кнопки мыши
    void enterEvent(QEvent * event) override; // При входе в область виджета
    void leaveEvent(QEvent * event) override; // При выходе из области виджета
private:
    QRubberBand * rubberBand_;
    QPoint origin_; // Положение левого верхнего угла прямоугольника
    bool isZoomed = false; // Флаг масштабированного изображения
    QCPRange rangeXAxis2_; // Диапазон по дополнительной горизонтальной оси
    QCPRange rangeYAxis2_; // Диапазон по дополнительной вертикальной оси
};

#endif // QCUSTOMPLOTZOOM_H
