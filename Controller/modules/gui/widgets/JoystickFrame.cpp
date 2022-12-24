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

void JoystickFrame::updateMessageToDisplay(CtrlToRadioCommandMessage msgToDisplay)
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
    QPen original = painter.pen();
    QPen pen;
    QColor white(255, 255, 255);
    QColor blue(0, 0, 200);

    int w = width();
    int h = height();

    pen.setColor(white);
    painter.setPen(pen);
    painter.drawText(10, 20, "L2");
    painter.fillRect(10, 30, 40, 20, white);
    pen.setColor(blue);
    painter.setPen(pen);
    painter.drawText(10 + 5, 30 + 17, QString::number(_msgToDisplay.l2_axis));

    pen.setColor(white);
    painter.setPen(pen);
    painter.drawText(10, 70, "L3 X");
    painter.fillRect(10, 80, 40, 20, white);
    pen.setColor(blue);
    painter.setPen(pen);
    painter.drawText(10 + 5, 80 + 17, QString::number(_msgToDisplay.l3_x_axis));

    pen.setColor(white);
    painter.setPen(pen);
    painter.drawText(10, 120, "L3 Y");
    painter.fillRect(10, 130, 40, 20, white);
    pen.setColor(blue);
    painter.setPen(pen);
    painter.drawText(10 + 5, 130 + 17, QString::number(_msgToDisplay.l3_y_axis));

    pen.setColor(white);
    painter.setPen(pen);
    painter.drawText(w - 50, 20, "R2");
    painter.fillRect(w - 50, 30, 40, 20, white);
    pen.setColor(blue);
    painter.setPen(pen);
    painter.drawText(w - 50 + 5, 30 + 17, QString::number(_msgToDisplay.r2_axis));

    pen.setColor(white);
    painter.setPen(pen);
    painter.drawText(w - 50, 70, "R3 X");
    painter.fillRect(w - 50, 80, 40, 20, white);
    pen.setColor(blue);
    painter.setPen(pen);
    painter.drawText(w - 50 + 5, 80 + 17, QString::number(_msgToDisplay.r3_x_axis));

    pen.setColor(white);
    painter.setPen(pen);
    painter.drawText(w - 50, 120, "R3 Y");
    painter.fillRect(w - 50, 130, 40, 20, white);
    pen.setColor(blue);
    painter.setPen(pen);
    painter.drawText(w - 50 + 5, 130 + 17, QString::number(_msgToDisplay.r3_y_axis));

    painter.setPen(original);

    int analogAxisRectWidth = w/3;
    int analogAxisRectHeight = h/3;
    int l3AnalogAxisX = 10;
    int r3AnalogAxisX = w - 10 - analogAxisRectWidth;
    int l3AnalogAxisY = h/2 + 20;
    int r3AnalogAxisY = h/2 + 20;

    paintAnalogRectangle("L3", l3AnalogAxisX, l3AnalogAxisY, analogAxisRectWidth, analogAxisRectHeight, _msgToDisplay.l3_x_axis, _msgToDisplay.l3_y_axis);
    paintAnalogRectangle("R3", r3AnalogAxisX, r3AnalogAxisY, analogAxisRectWidth, analogAxisRectHeight, _msgToDisplay.r3_x_axis, _msgToDisplay.r3_y_axis);
}

void JoystickFrame::paintAnalogRectangle(QString analogAxisName, int x0, int y0, int w, int h, int8_t xAxis, int8_t yAxis)
{
    QPainter painter(this);

    painter.drawText(x0 + w / 2 - 7, y0 - 10, analogAxisName);
    painter.drawRect(x0, y0, w, h);

    int valuesSpan = std::numeric_limits<int8_t>::max() - std::numeric_limits<int8_t>::min();
    double xPercentage = ((double) xAxis - (double) std::numeric_limits<int8_t>::min()) / (double) valuesSpan;
    double yPercentage = ((double) yAxis - (double) std::numeric_limits<int8_t>::min()) / (double) valuesSpan;

    int x = x0 + xPercentage * w;
    int y = y0 + yPercentage * h;

    painter.drawLine(x - 10, y, x + 10, y);
    painter.drawLine(x, y - 10, x, y + 10);
}
