#include <QRubberBand>
#include "gui/QCustomPlotZoom.h"

// ---- Графическое окно ---------------------------------------------------------------------------------------

// Конструктор
QCustomPlotZoom::QCustomPlotZoom(QWidget * parent)
    : QCustomPlot(parent),
    rubberBand_(new QRubberBand(QRubberBand::Rectangle, this))
{}

// Деструктор
QCustomPlotZoom::~QCustomPlotZoom()
{
    delete rubberBand_;
}

// При нажатии кнопки мыши
void QCustomPlotZoom::mousePressEvent(QMouseEvent * event)
{
    // Выделение области
    if (event->button() == Qt::LeftButton)
    {
        origin_ = event->pos(); // Положение левого верхнего угла
        rubberBand_->setGeometry(QRect(origin_, QSize())); // Формирование области выделения
        rubberBand_->show(); // Включение отображения области выделения
    }
    // Снятие выделения области
    if (event->button() == Qt::RightButton){
        rescaleAxes(); // Сброс выделения
        replot(); // Обновление графика
    }
    QCustomPlot::mousePressEvent(event);
}

// При сдвиге мыши
void QCustomPlotZoom::mouseMoveEvent(QMouseEvent * event)
{
    // Если область выделена
    if (rubberBand_->isVisible())
        rubberBand_->setGeometry(QRect(origin_, event->pos()).normalized()); // Изменение размера области вслед за курсором мыши
    QCustomPlot::mouseMoveEvent(event);
}

// При отжатии кнопки мыши
void QCustomPlotZoom::mouseReleaseEvent(QMouseEvent * event)
{
    // Если область выделена
    if (rubberBand_->isVisible())
    {
        const QRect & zoomRect = rubberBand_->geometry(); // Размер области выделения
        // Получение координат граничных точек области
        int xp1, yp1, xp2, yp2;
        zoomRect.getCoords(&xp1, &yp1, &xp2, &yp2);
        auto x1 = xAxis->pixelToCoord(xp1);
        auto x2 = xAxis->pixelToCoord(xp2);
        auto y1 = yAxis->pixelToCoord(yp1);
        auto y2 = yAxis->pixelToCoord(yp2);
        // Изменение диапазона осей по этой области
        xAxis->setRange(x1, x2);
        yAxis->setRange(y1, y2);
        rubberBand_->hide(); // Скрытие прямоугольника выделения
        replot(); // Обновление графика
    }
    QCustomPlot::mouseReleaseEvent(event);
}

// При входе в область виджета
void QCustomPlotZoom::enterEvent(QEvent * /* event */){
    QApplication::setOverrideCursor(Qt::CrossCursor); // Изменение формы курсора
}

 // При выходе из области виджета
void QCustomPlotZoom::leaveEvent(QEvent * /* event */){
    QApplication::restoreOverrideCursor(); // Возвращение исходной формы курсора
}

// -------------------------------------------------------------------------------------------------------------
