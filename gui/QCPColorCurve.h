#ifndef QCPCOLORCURVE_H
#define QCPCOLORCURVE_H

#include "include/QCustomPlot.h"

// Построение кривой с заданным распределением цветов

class QCPColorCurve : public QCPCurve
{
public:
    QCPColorCurve(QCPAxis *keyAxis, QCPAxis *valueAxis);
    virtual ~QCPColorCurve() = default;
    void setData(const QVector<double> & keys, const QVector<double> & values, const QVector<QColor> & colors);
protected:
    virtual void drawScatterPlot(QCPPainter *painter, const QVector<QPointF> &points, const QCPScatterStyle &style) const;
private:
    QVector<QColor> colors_;
};

#endif // QCPCOLORCURVE_H
