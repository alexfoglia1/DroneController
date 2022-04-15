#include "modules/gui/widgets/JoystickFrame.h"

#include <QPainter>
#include <limits>

JoystickFrame::JoystickFrame(QWidget* parent) : QFrame(parent)
{
    _msgToDisplay.l2_axis = 0;
    _msgToDisplay.r2_axis = 0;
    _msgToDisplay.l3_x_axis = 0;
    _msgToDisplay.l3_y_axis = 0;
    _msgToDisplay.r3_x_axis = 0;
    _msgToDisplay.r3_y_axis = 0;

    setGeometry(5, parent->height()/2, parent->width() - 10, parent->height()/2 - 10);
    setStyleSheet("background-color: blue;"
                  "color: white;");
}

void JoystickFrame::updateMessageToDisplay(CtrlToRadioCommand msgToDisplay)
{
    bool updated =
       (msgToDisplay.l2_axis != _msgToDisplay.l2_axis     ||
        msgToDisplay.r2_axis != _msgToDisplay.r2_axis     ||
        msgToDisplay.l3_x_axis != _msgToDisplay.l3_x_axis ||
        msgToDisplay.r3_y_axis != _msgToDisplay.r3_y_axis ||
        msgToDisplay.r3_x_axis != _msgToDisplay.r3_x_axis ||
        msgToDisplay.r3_y_axis != _msgToDisplay.r3_y_axis);

    _msgToDisplay = msgToDisplay;

    if (updated)
    {
        repaint();
    }
}

void JoystickFrame::paintEvent(QPaintEvent* paintEvent)
{
    QFrame::paintEvent(paintEvent);

    QPainter painter(this);

    int w = width();
    int h = height();
    painter.drawText(10, 20, QString("L2: %1").arg(_msgToDisplay.l2_axis));
    painter.drawText(10, 40, QString("R2: %1").arg(_msgToDisplay.r2_axis));
    painter.drawText(10, 60, QString("L3 X: %1").arg(_msgToDisplay.l3_x_axis));
    painter.drawText(10, 80, QString("L3 Y: %1").arg(_msgToDisplay.l3_y_axis));
    painter.drawText(10, 100, QString("R3 X: %1").arg(_msgToDisplay.r3_x_axis));
    painter.drawText(10, 120, QString("R3 Y: %1").arg(_msgToDisplay.r3_y_axis));

    int analogAxisRectWidth = w/3;
    int analogAxisRectHeight = h/3;
    int l3AnalogAxisX = 10;
    int r3AnalogAxisX = w - 10 - analogAxisRectWidth;
    int l3AnalogAxisY = h/2;
    int r3AnalogAxisY = h/2;

    paintAnalogRectangle("L3", l3AnalogAxisX, l3AnalogAxisY, analogAxisRectWidth, analogAxisRectHeight, _msgToDisplay.l3_x_axis, _msgToDisplay.l3_y_axis);
    paintAnalogRectangle("R3", r3AnalogAxisX, r3AnalogAxisY, analogAxisRectWidth, analogAxisRectHeight, _msgToDisplay.r3_x_axis, _msgToDisplay.r3_y_axis);
}

void JoystickFrame::paintAnalogRectangle(QString analogAxisName, int x0, int y0, int w, int h, int8_t xAxis, int8_t yAxis)
{
    QPainter painter(this);

    painter.drawText(x0, y0 - 10, analogAxisName);
    painter.drawRect(x0, y0, w, h);

    int valuesSpan = std::numeric_limits<int8_t>::max() - std::numeric_limits<int8_t>::min();
    double xPercentage = ((double) xAxis - (double) std::numeric_limits<int8_t>::min()) / (double) valuesSpan;
    double yPercentage = ((double) yAxis - (double) std::numeric_limits<int8_t>::min()) / (double) valuesSpan;

    int x = x0 + xPercentage * w;
    int y = y0 + yPercentage * h;

    painter.drawLine(x - 10, y, x + 10, y);
    painter.drawLine(x, y - 10, x, y + 10);
}
