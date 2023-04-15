#include "CompassWidget.h"
#include <qpainter.h>
#include <math.h>

CompassWidget::CompassWidget(QWidget* parent) : QLabel(parent)
{
	_pixmap = nullptr;
	_yaw = 0.0f;
}

void CompassWidget::setGeometry(const QRect& r)
{
	QLabel::setGeometry(r);

	if (_pixmap)
	{
		delete _pixmap;
	}

	_pixmap = new QPixmap(r.width(), r.height());
}

void CompassWidget::setYaw(float yaw)
{
	_yaw = yaw;
	repaint();
}


void CompassWidget::paintEvent(QPaintEvent* event)
{
	QLabel::paintEvent(event);

	QRect r = rect();
	QPoint center = QPoint(r.x() + r.width() / 2, r.y() + r.height() / 2);
	QPainter compassPainter(_pixmap);
	compassPainter.setFont(QFont("Courier New", 16));

	_pixmap->fill(Qt::darkGray);
	int ray = r.height() / 2 - 20;

	QPen whitePen(QColor(0xff, 0xff, 0xff), 2.0);
	QPen thinWhitePen(QColor(0xff, 0xff, 0xff), 1.0);
	QPen redPen(QColor(0xff, 0x00, 0x00), 2.0);
	compassPainter.setPen(whitePen);
	compassPainter.drawEllipse(center, r.width() / 2, r.width() / 2);
	
	float northHeading = 2 * 3.1415 - _yaw;
	float northHeadingPixmap = northHeading - 3.1415 / 2.0f;
	
	compassPainter.setPen(redPen);
	float nx = center.x() + cos(northHeadingPixmap) * (ray + 10);
	float ny = center.y() + sin(northHeadingPixmap) * (ray + 10);

	compassPainter.drawText(nx - 8, ny + 7, "N");

	for (int heading = 0; heading < 360; heading += 9)
	{
		compassPainter.setPen(thinWhitePen);
		float cur_heading_radians = (float)heading / 57.295780f;
		float outer_point_x = center.x() + cos(cur_heading_radians) * ray;
		float outer_point_y = center.y() + sin(cur_heading_radians) * ray;
		float inner_point_x = center.x() + cos(cur_heading_radians) * (ray - 10);
		float inner_point_y = center.y() + sin(cur_heading_radians) * (ray - 10);

		compassPainter.drawLine(inner_point_x, inner_point_y, outer_point_x, outer_point_y);
	}



	compassPainter.end();

	QPainter painter(this);
	painter.drawPixmap(r, *_pixmap);
	painter.end();
}