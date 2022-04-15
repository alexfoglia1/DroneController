#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "modules/radio/proto.h"

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
    const int R2_AXIS = 5;
    const int L2_AXIS = 2;
    const int X_BUTTON = 1;
    const int R1_BUTTON = 5;
    const int L1_BUTTON = 4;
    const int PS_BUTTON = 12;
    const int L3_VERTICAL_AXIS = 1;
    const int L3_HORIZONTAL_AXIS = 0;
    const int R3_VERTICAL_AXIS = 4;
    const int R3_HORIZONTAL_AXIS = 3;
    const int JOY_DEAD_CENTER_ZONE = 2000;

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
