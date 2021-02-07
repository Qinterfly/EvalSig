#include "QCPColorCurve.h"

// ---- Построение кривой с заданным распределением цветов -----------------------------------------------------

QCPColorCurve::QCPColorCurve(QCPAxis *keyAxis, QCPAxis *valueAxis) : QCPCurve(keyAxis, valueAxis) {}

// Задание данных в виде координат точек и цветов
void QCPColorCurve::setData(const QVector<double> & keys, const QVector<double> & values, const QVector<QColor> & colors){
    if (values.size() != colors.size()) return;
    colors_ = colors;
    QCPCurve::setData(keys, values);
}

// Построение точек
void QCPColorCurve::drawScatterPlot(QCPPainter * painter, const QVector<QPointF> & points, const QCPScatterStyle & style) const {
    applyScattersAntialiasingHint(painter);
    int nPoints = points.size();
    for (int i = 0; i < nPoints; ++i)
        if (!qIsNaN(points.at(i).x()) && !qIsNaN(points.at(i).y())){
            painter->setPen(colors_[i]);
            style.drawShape(painter, points.at(i));
        }
}

// -------------------------------------------------------------------------------------------------------------
