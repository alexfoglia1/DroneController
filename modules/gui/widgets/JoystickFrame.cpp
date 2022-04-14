#include "modules/gui/widgets/JoystickFrame.h"

#include <QPainter>

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

void JoystickFrame::updateMessageToDisplay(CtrlToRadioMsg msgToDisplay)
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

    painter.drawText(10, 20, QString("L2: %1").arg(_msgToDisplay.l2_axis));
    painter.drawText(10, 40, QString("R2: %1").arg(_msgToDisplay.r2_axis));
    painter.drawText(10, 60, QString("L3 X: %1").arg(_msgToDisplay.l3_x_axis));
    painter.drawText(10, 80, QString("L3 Y: %1").arg(_msgToDisplay.l3_y_axis));
    painter.drawText(10, 100, QString("R3 X: %1").arg(_msgToDisplay.r3_x_axis));
    painter.drawText(10, 120, QString("R3 Y: %1").arg(_msgToDisplay.r3_y_axis));
}
