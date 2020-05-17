#include <QRubberBand>
#include "gui/QCustomPlotZoom.h"

// ---- Графическое окно ---------------------------------------------------------------------------------------

// Конструктор
QCustomPlotZoom::QCustomPlotZoom(QWidget * parent)
    : QCustomPlot(parent),
    rubberBand_(new QRubberBand(QRubberBand::Rectangle, this)),
    keyXAxis_(xAxis), keyYAxis_(yAxis)
{ }

// Деструктор
QCustomPlotZoom::~QCustomPlotZoom()
{
    delete rubberBand_;
}

// Переключение возможности масштабирования
void QCustomPlotZoom::setZoomEnabled(bool enabled) {
    isZoomEnabled_ = enabled;
}

// Установка осей для масштабирования
void QCustomPlotZoom::setKeyAxes(QPair<int, int> indexes){
    indexes.first == 1 ? keyXAxis_ = xAxis : keyXAxis_ = xAxis2;
    indexes.second == 1 ? keyYAxis_ = yAxis : keyYAxis_ = yAxis2;
}

// При нажатии кнопки мыши
void QCustomPlotZoom::mousePressEvent(QMouseEvent * event)
{
    showCoordTag(event); // Отображение курсорной подсказки
    if ( !isZoomEnabled_ ) return;
    // Выделение области
    if (event->button() == Qt::LeftButton)
    {
        origin_ = event->pos(); // Положение левого верхнего угла
        if ( !isZoomed_ ){ // Если масштабирование первое
            rangeXAxis_ = keyXAxis_->range(); // Диапазон по горизонтальной оси
            rangeYAxis_ = keyYAxis_->range(); // Диапазон по вертикальной оси
            isZoomed_ = true; // Изображение смаштабировано
        }
        rubberBand_->setGeometry(QRect(origin_, QSize())); // Формирование области выделения
        rubberBand_->show(); // Включение отображения области выделения
    }
    // Снятие выделения области
    if (event->button() == Qt::RightButton){
        rescaleAxes(); // Сброс выделения
        keyXAxis_->setRange(rangeXAxis_); // Возврат к диапазонам до масштабирования по X
        keyYAxis_->setRange(rangeYAxis_); // Возврат к диапазонам до масштабирования по Y
        isZoomed_ = false; // Сброс флага масштабированного изображения
        replot(); // Обновление графика
    }
    QCustomPlot::mousePressEvent(event);
}

// При сдвиге мыши
void QCustomPlotZoom::mouseMoveEvent(QMouseEvent * event)
{
    // Если область выделена
    if (rubberBand_->isVisible()){
        rubberBand_->setGeometry(QRect(origin_, event->pos()).normalized()); // Изменение размера области вслед за курсором мыши
        showCoordTag(event); // Отображение курсорной подсказки
    }
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
        // Основные координаты
        auto x1m = xAxis->pixelToCoord(xp1);
        auto x2m = xAxis->pixelToCoord(xp2);
        auto y1m = yAxis->pixelToCoord(yp1);
        auto y2m = yAxis->pixelToCoord(yp2);
        // Дополнительные координаты
        auto x1a = xAxis2->pixelToCoord(xp1);
        auto x2a = xAxis2->pixelToCoord(xp2);
        auto y1a = yAxis2->pixelToCoord(yp1);
        auto y2a = yAxis2->pixelToCoord(yp2);
        // Изменение диапазона осей по этой области
            // По основным осям
        xAxis->setRange(x1m, x2m);
        yAxis->setRange(y1m, y2m);
            // По дополнительным осям
        xAxis2->setRange(x1a, x2a);
        yAxis2->setRange(y1a, y2a);
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

// Отображение курсорной подсказки
void QCustomPlotZoom::showCoordTag(QMouseEvent * event){
    // Смещение подсказки от положения курсора
    static const double SHIFT_TAG_X = 0.00 * screen()->size().width();
    static const double SHIFT_TAG_Y = -0.01 * screen()->size().height();
    // Параметры отображения подсказки
    bool isShownTag = true;
    QPoint cursorPos = event->pos();
    // Отображение подсказки с координатами
    if ( isShownTag ){
        double x = keyXAxis_->pixelToCoord(cursorPos.x());
        double y = keyYAxis_->pixelToCoord(cursorPos.y());
        QString textTag = QString("(%1, %2)").arg(QString::number(x, 'g', 4)).arg(QString::number(y, 'g', 4));
        cursorPos = QPoint(keyXAxis_->coordToPixel(x) + SHIFT_TAG_X, keyYAxis_->coordToPixel(y) + SHIFT_TAG_Y);
        cursorPos = mapToGlobal(cursorPos); // Координаты курсора в системе координат экрана
        QToolTip::showText(cursorPos, textTag, this, rect(), 1000);
    }
}

// -------------------------------------------------------------------------------------------------------------
