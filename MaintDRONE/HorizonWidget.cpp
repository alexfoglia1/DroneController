#include "HorizonWidget.h"
#include <qpainter.h>
#include <math.h>

HorizonWidget::HorizonWidget(QWidget* parent) : QLabel(parent)
{
	_pixmap = nullptr;
	_roll = 0.0f;
	_pitch = 0.0f;
}

void HorizonWidget::setGeometry(const QRect& r)
{
	QLabel::setGeometry(r);

	if (_pixmap)
	{
		delete _pixmap;
	}

	_pixmap = new QPixmap(r.width(), r.height());
}

void HorizonWidget::setRollPitch(float roll, float pitch)
{
	_roll = roll;
	_pitch = pitch;

	repaint();
}


void HorizonWidget::paintEvent(QPaintEvent* event)
{
	QLabel::paintEvent(event);

	QRect r = rect();
	QPoint center = QPoint(r.x() + r.width() / 2, r.y() + r.height() / 2);
	QPainter horizonPainter(_pixmap);
	_pixmap->fill(Qt::blue);
	int ray = r.height() / 2;

	float m_Pitch = _pitch < -3.1415f / 2.0f ? -3.1415f / 2.0f :
		_pitch > 3.1415f / 2.0f ? 3.1415f / 2.0f :
		_pitch;
	float normPitch = (_pitch + 3.1415f) / (2 * 3.1415f);

	int horizonHeight = r.height() * normPitch;
	horizonPainter.fillRect(r.x(), horizonHeight, r.width(), r.height() - horizonHeight, QColor(128, 0, 0));

	QPen whitePen(QColor(0xff, 0xff, 0xff), 2.0);
	horizonPainter.setPen(whitePen);
	horizonPainter.drawEllipse(center, r.width() / 2, r.width() / 2);
	
	int x0_roll = center.x() + ray / 2.0f * cos(_roll);
	int y0_roll = center.y() + ray / 2.0f * sin(_roll);
	int xf_roll = center.x() - ray / 2.0f * cos(_roll);
	int yf_roll = center.y() - ray / 2.0f * sin(_roll);

	horizonPainter.drawLine(x0_roll, y0_roll, xf_roll, yf_roll);

	horizonPainter.end();

	QPainter painter(this);
	painter.drawPixmap(r, *_pixmap);
	painter.end();
}