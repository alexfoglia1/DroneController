#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "modules/radio/proto.h"
#include "modules/settings/settings.h"

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <limits>
#include <string.h>
#include <QThread>
#include <signal.h>
#include <SDL.h>
#include <QMutex>

class Joystick : public QThread
{
    Q_OBJECT

public:

    int R2_AXIS;
    int L2_AXIS;
    int X_BUTTON;
    int SQUARE_BUTTON;
    int R1_BUTTON;
    int L1_BUTTON;
    int PS_BUTTON;
    int L3_VERTICAL_AXIS;
    int L3_HORIZONTAL_AXIS;
    int R3_VERTICAL_AXIS;
    int R3_HORIZONTAL_AXIS;
    int JOY_DEAD_CENTER_ZONE;

    enum js_thread_state_t
    {
        IDLE,
        OPERATIVE,
        EXIT
    };

    Joystick();
    void updateState(js_thread_state_t newState);

signals:
    void jsConnected(bool connected);
    void btnPressed(int btn);
    void msgOut(CtrlToRadioCommand msgOut);

protected:
    void run() override;

private:
    SDL_Joystick *js;
    CtrlToRadioCommand _msgOut;

    int min_js_axis_value;
    int max_js_axis_value;
    js_thread_state_t act_state;
    QMutex statesMutex;

    void updateMsgOut(SDL_Event event);
    bool init_joystick();
    int8_t map_js_axis_value_int8(int js_axis_value);
    uint8_t map_js_axis_value_uint8(int js_axis_value);

};


#endif //JOYSTICK_H
