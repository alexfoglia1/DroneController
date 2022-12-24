#ifndef DRONE_FRAME_H
#define DRONE_FRAME_H

#include "modules/radio/proto.h"

#include <QFrame>

class DroneFrame : public QFrame
{
    Q_OBJECT
public:
    explicit DroneFrame(QWidget* parent = nullptr);
    void updateMessageToDisplay(DroneToRadioResponseMessage msgToDisplay);
    void paintEvent(QPaintEvent* paintEvent);

private:
    DroneToRadioResponseMessage _msgToDisplay;

};

#endif //DRONE_FRAME_H
