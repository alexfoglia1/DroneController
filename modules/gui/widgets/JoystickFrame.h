#ifndef JOYSTICK_FRAME_H
#define JOYSTICK_FRAME_H

#include "modules/radio/proto.h"

#include <QFrame>

class JoystickFrame : public QFrame
{
    Q_OBJECT
public:
    explicit JoystickFrame(QWidget* parent = nullptr);
    void updateMessageToDisplay(CtrlToRadioCommand msgToDisplay);
    void paintEvent(QPaintEvent* paintEvent);

private:
    CtrlToRadioCommand _msgToDisplay;

    void paintAnalogRectangle(QString analogAxisName, int x0, int y0, int w, int h, int8_t xAxis, int8_t yAxis);

};

#endif //JOYSTICK_FRAME_H
