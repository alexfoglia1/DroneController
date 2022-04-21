#include "modules/gui/widgets/DroneFrame.h"

#include <QPainter>
#include <QColor>

#ifdef WIN32
#define _USE_MATH_DEFINES
#endif
#include <math.h>

DroneFrame::DroneFrame(QWidget* parent) : QFrame(parent)
{
    _msgToDisplay.baro_altitude = 0;
    _msgToDisplay.heading = 0;
    _msgToDisplay.roll = 0;
    _msgToDisplay.pitch = 0;
    _msgToDisplay.motor1_speed = 0;
    _msgToDisplay.motor2_speed = 0;
    _msgToDisplay.motor3_speed = 0;
    _msgToDisplay.motor4_speed = 0;
    _msgToDisplay.motors_armed = 0;

    setGeometry(5, parent->height()/3, parent->width() - 10, parent->height() - 10 - parent->height()/3);
    setStyleSheet("background-color: blue;"
                  "color: white;");
}

void DroneFrame::updateMessageToDisplay(DroneToRadioResponseMessage msgToDisplay)
{
    bool updated =
       (msgToDisplay.baro_altitude != _msgToDisplay.baro_altitude ||
        msgToDisplay.heading != _msgToDisplay.heading             ||
        msgToDisplay.roll != _msgToDisplay.roll                   ||
        msgToDisplay.pitch != _msgToDisplay.pitch                 ||
        msgToDisplay.motor1_speed != _msgToDisplay.motor1_speed   ||
        msgToDisplay.motor2_speed != _msgToDisplay.motor2_speed   ||
        msgToDisplay.motor3_speed != _msgToDisplay.motor3_speed   ||
        msgToDisplay.motor4_speed != _msgToDisplay.motor4_speed   ||
        msgToDisplay.motors_armed != _msgToDisplay.motors_armed);

    _msgToDisplay = msgToDisplay;

    if (updated)
    {
        repaint();
    }
}

void DroneFrame::paintEvent(QPaintEvent* paintEvent)
{
    QFrame::paintEvent(paintEvent);

    QPainter painter(this);
    QPen pen;
    QColor white(255, 255, 255);
    QColor red(255, 0, 0);
    QColor green(0, 255, 0);

    int w = width();
    int h = height();

    /** Draw sprite of drone **/
    int centralWidth = 40;
    int centralHeight = 40;
    int centralSpriteX0 = w/6 + centralWidth/2;
    int centralSpriteY0 = h/3;
    int motorSpriteSize = 10;

    int spriteCenterX = centralSpriteX0 + centralWidth/2;
    int spriteCenterY = centralSpriteY0 + centralHeight/2;
    int arrowX0 = spriteCenterX;
    int arrowY0 = spriteCenterY - centralHeight / 2 - 10;
    int arrowLen = 80;
    int arrowXf = arrowX0;
    int arrowYf = arrowY0 - arrowLen;

    int wingLength = 140;
    int wingX0 = spriteCenterX;
    int wingY0 = spriteCenterY;

    int wing1Xf = wingX0 + (wingLength) * cos(5 * M_PI/4);
    int wing1Yf = wingY0 + (wingLength) * sin(5 * M_PI/4);
    int wing2Xf = wingX0 + (wingLength) * cos(3 * M_PI/4);
    int wing2Yf = wingY0 + (wingLength) * sin(3 * M_PI/4);
    int wing3Xf = wingX0 + (wingLength) * cos(1 * M_PI/4);
    int wing3Yf = wingY0 + (wingLength) * sin(1 * M_PI/4);
    int wing4Xf = wingX0 + (wingLength) * cos(7 * M_PI/4);
    int wing4Yf = wingY0 + (wingLength) * sin(7 * M_PI/4);

    /** Draw central drone sprite **/
    pen.setWidth(5);
    pen.setColor(white);
    painter.setPen(pen);
    painter.fillRect(centralSpriteX0, centralSpriteY0, centralWidth, centralHeight, pen.color());

    /** Draw drone wings **/
    painter.drawLine(wingX0, wingY0, wing1Xf, wing1Yf);
    painter.drawLine(wingX0, wingY0, wing2Xf, wing2Yf);
    painter.drawLine(wingX0, wingY0, wing3Xf, wing3Yf);
    painter.drawLine(wingX0, wingY0, wing4Xf, wing4Yf);

    /** Draw drone motors **/
    pen.setColor(_msgToDisplay.motors_armed == 0 ? red : green);
    painter.setPen(pen);

    painter.drawEllipse(wing1Xf - 2 * motorSpriteSize, wing1Yf - 2 * motorSpriteSize,  4 * motorSpriteSize, 4 * motorSpriteSize);
    painter.drawEllipse(wing2Xf - 2 * motorSpriteSize, wing2Yf - 2 * motorSpriteSize,  4 * motorSpriteSize, 4 * motorSpriteSize);
    painter.drawEllipse(wing3Xf - 2 * motorSpriteSize, wing3Yf - 2 * motorSpriteSize,  4 * motorSpriteSize, 4 * motorSpriteSize);
    painter.drawEllipse(wing4Xf - 2 * motorSpriteSize, wing4Yf - 2 * motorSpriteSize,  4 * motorSpriteSize, 4 * motorSpriteSize);

    /** Write drone motors status **/
    pen.setWidth(1);
    painter.setPen(pen);
    painter.drawText(wing1Xf - 2 * motorSpriteSize, wing1Yf + 2 * motorSpriteSize - 50, QString("M1: %1").arg(_msgToDisplay.motor1_speed));
    painter.drawText(wing2Xf - 2 * motorSpriteSize, wing2Yf + 2 * motorSpriteSize + 20, QString("M2: %1").arg(_msgToDisplay.motor2_speed));
    painter.drawText(wing3Xf - 2 * motorSpriteSize, wing3Yf - 2 * motorSpriteSize + 60, QString("M3: %1").arg(_msgToDisplay.motor3_speed));
    painter.drawText(wing4Xf - 2 * motorSpriteSize, wing4Yf - 2 * motorSpriteSize - 10, QString("M4: %1").arg(_msgToDisplay.motor4_speed));

    /** Draw drone direction arrow **/
    pen.setWidth(3);
    pen.setColor(white);
    painter.setPen(pen);
    painter.drawLine(arrowX0, arrowY0, arrowXf, arrowYf);
    painter.drawLine(arrowXf, arrowYf, arrowXf + (arrowLen / 5) * cos(M_PI/4), arrowYf + (arrowLen / 5) * sin(M_PI/4));
    painter.drawLine(arrowXf, arrowYf, arrowXf + (arrowLen / 5) * cos(3 * M_PI/4), arrowYf + (arrowLen / 5) * sin(3 * M_PI/4));

}
