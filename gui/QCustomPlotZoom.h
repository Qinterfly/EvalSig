#ifndef QCUSTOMPLOTZOOM_H
#define QCUSTOMPLOTZOOM_H

#include <QPoint>
#include "include/QCustomPlot.h"

class QRubberBand;
class QMouseEvent;
class QWidget;

enum StateCoordTag { HIDE, SHOW_MAIN, SHOW_ADDITIONAL }; // Опции отображения курсорной подсказки

struct QCustomPlotZoom : public QCustomPlot {
    Q_OBJECT
public:
    QCustomPlotZoom(QWidget * parent = nullptr);
    ~QCustomPlotZoom() override;
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
    QCPRange rangeXAxis2_; // Диапазон по дополнительной горизонтальной оси
    QCPRange rangeYAxis2_; // Диапазон по дополнительной вертикальной оси
    StateCoordTag stateCoordTag_ = SHOW_ADDITIONAL; // По умолчанию скрывать подписи точек
};

#endif // QCUSTOMPLOTZOOM_H
