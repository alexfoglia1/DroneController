#ifndef JOYSTICK_FRAME_H
#define JOYSTICK_FRAME_H

#include "modules/radio/proto.h"

#include <QFrame>

class JoystickFrame : public QFrame
{
    Q_OBJECT
public:
    explicit JoystickFrame(QWidget* parent = nullptr);
    void updateMessageToDisplay(CtrlToRadioMsg msgToDisplay);
    void paintEvent(QPaintEvent* paintEvent);

private:
    CtrlToRadioMsg _msgToDisplay;

};

#endif //JOYSTICK_FRAME_H
