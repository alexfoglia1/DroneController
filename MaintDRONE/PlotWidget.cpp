#include "PlotWidget.h"
#include <qpainter.h>

PlotWidget::PlotWidget(QWidget* parent) : QLabel(parent)
{
	pixmap = nullptr;
	track1_range = 20;
	track2_range = 20;
	track3_range = 20;

	t_range = 100;
}

void PlotWidget::setGeometry(const QRect& r)
{
	QLabel::setGeometry(r);

	if (pixmap)
	{
		delete pixmap;
	}

	pixmap = new QPixmap(r.width(), r.height());
}

void PlotWidget::setRange(PlotTrack track, float range)
{
	if (range < 1) range = 1;

	float* ranges[3] = { &track1_range, &track2_range, &track3_range };
	*ranges[static_cast<int>(track)] = range;
	repaint();
}


void PlotWidget::addValue(PlotTrack track, float val)
{

	switch (track)
	{
	case PlotTrack::PLOT_TRACK_1:
	{
		if (track1.size() < t_range)
		{
			track1.push_back(val);
		}
		else
		{
			for (int i = 1; i < track1.size(); i++)
			{
				track1[i - 1] = track1[i];
			}

			track1.pop_back();
			track1.push_back(val);
		}

		if (fabs(val) > track1_range / 2)
		{
			track1_range *= 2;
		}
		break;
		}
		case PlotTrack::PLOT_TRACK_2:
		{
			if (track2.size() < t_range)
			{
				track2.push_back(val);
			}
			else
			{
				for (int i = 1; i < track2.size(); i++)
				{
					track2[i - 1] = track2[i];
				}

				track2.pop_back();
				track2.push_back(val);
			}
			if (fabs(val) > track2_range / 2)
			{
				track2_range *= 2;
			}
			break;
		}
		case PlotTrack::PLOT_TRACK_3:
		{
			if (track3.size() < t_range)
			{
				track3.push_back(val);
			}
			else
			{
				for (int i = 1; i < track3.size(); i++)
				{
					track3[i - 1] = track3[i];
				}

				track3.pop_back();
				track3.push_back(val);
			}
			if (fabs(val) > track3_range / 2)
			{
				track3_range *= 2;
			}
			break;
		}
	}
}

void PlotWidget::paintEvent(QPaintEvent* event)
{
	QLabel::paintEvent(event);

	QRect r = rect();

	QPainter plotPainter(pixmap);
	pixmap->fill(Qt::black);
	plotPainter.setPen(QColor(0xff, 0xff, 0xff));
	plotPainter.drawLine(OFFSET_X_PX, 0, OFFSET_X_PX, r.height());
	plotPainter.drawLine(0, r.height() / 2, r.width(), r.height() / 2);
	std::vector<float>* tracks[3] = { &track1, &track2, &track3 };
	float* ranges[3] = { &track1_range, &track2_range, &track3_range };

	for (int y = 5 * OFFSET_X_PX; y < r.height(); y += 5 * OFFSET_X_PX)
	{
		plotPainter.setPen(QColor(0xff, 0xff, 0xff));
		plotPainter.drawLine(OFFSET_X_PX, y, r.width(), y);
		for (int track = 0; track < 3; track++)
		{
			std::vector<float> currentTrack = *tracks[static_cast<int>(track)];
			float currentRange = *ranges[static_cast<int>(track)];
			QColor currentColor = trackColors[static_cast<int>(track)];
			plotPainter.setPen(currentColor);
			float scaleY = r.height() / currentRange;

			float f = (r.height() / 2 - y) / scaleY;

			plotPainter.drawText(QPointF(OFFSET_X_PX + 5, y + (track - 3) * 12), QString::number(f));
		}
	}

	float scaleX = (r.width() - OFFSET_X_PX) / t_range;
	
	
	for (int track = 0; track < 3; track++)
	{
		std::vector<float> currentTrack = *tracks[static_cast<int>(track)];
		float currentRange = *ranges[static_cast<int>(track)];
		QColor currentColor = trackColors[static_cast<int>(track)];

		plotPainter.setPen(currentColor);
		float scaleY = r.height() / currentRange;
		if (currentTrack.size() > 1)
		{
			for (int i = 1; i < currentTrack.size(); i++)
			{
				float t0 = (i - 1);
				float t1 = (i);
				float f0 = currentTrack.at(i - 1);
				float f1 = currentTrack.at(i);

				int x0 = OFFSET_X_PX + (int)t0 * scaleX;
				int y0 = r.height() / 2 - f0 * scaleY;

				int x1 = OFFSET_X_PX + (int)t1 * scaleX;
				int y1 = r.height() / 2 - f1 * scaleY;
				plotPainter.drawLine(x0, y0, x1, y1);
			}
		}
	}
	

	plotPainter.end();

	QPainter painter(this);
	painter.drawPixmap(r, *pixmap);
	painter.end();
}