#include "modules/joystick/joystick.h"

Joystick::Joystick()
{
    js = nullptr;
    min_js_axis_value = -32767;
    max_js_axis_value = 32767;
    act_state = IDLE;

    /** Apply settings **/
    R2_AXIS = Settings::instance()->getAttribute(Settings::Attribute::JOYSTICK_R2_AXIS).toInt();
    L2_AXIS = Settings::instance()->getAttribute(Settings::Attribute::JOYSTICK_L2_AXIS).toInt();
    X_BUTTON = Settings::instance()->getAttribute(Settings::Attribute::JOYSTICK_BTN_CROSS).toInt();
    SQUARE_BUTTON = Settings::instance()->getAttribute(Settings::Attribute::JOYSTICK_BTN_SQUARE).toInt();
    CIRCLE_BUTTON = Settings::instance()->getAttribute(Settings::Attribute::JOYSTICK_BTN_CIRCLE).toInt();
    TRIANGLE_BUTTON = Settings::instance()->getAttribute(Settings::Attribute::JOYSTICK_BTN_TRIANGLE).toInt();
    R1_BUTTON = Settings::instance()->getAttribute(Settings::Attribute::JOYSTICK_BTN_R1).toInt();
    L1_BUTTON = Settings::instance()->getAttribute(Settings::Attribute::JOYSTICK_BTN_L1).toInt();
    PS_BUTTON = Settings::instance()->getAttribute(Settings::Attribute::JOYSTICK_BTN_PS).toInt();
    L3_VERTICAL_AXIS = Settings::instance()->getAttribute(Settings::Attribute::JOYSTICK_L3_Y_AXIS).toInt();
    L3_HORIZONTAL_AXIS = Settings::instance()->getAttribute(Settings::Attribute::JOYSTICK_L3_X_AXIS).toInt();
    R3_VERTICAL_AXIS = Settings::instance()->getAttribute(Settings::Attribute::JOYSTICK_R3_Y_AXIS).toInt();
    R3_HORIZONTAL_AXIS = Settings::instance()->getAttribute(Settings::Attribute::JOYSTICK_R3_X_AXIS).toInt();
    JOY_DEAD_CENTER_ZONE = Settings::instance()->getAttribute(Settings::Attribute::JOYSTICK_DEAD_CENTER_ZONE).toInt();


    _msgOut.msg_id = CTRL_TO_RADIO_CMD_ID;
    _msgOut.l2_axis = 0;
    _msgOut.r2_axis = 0;
    _msgOut.l3_x_axis = 0;
    _msgOut.l3_y_axis = 0;
    _msgOut.r3_x_axis = 0;
    _msgOut.r3_y_axis = 0;
}

void Joystick::updateState(js_thread_state_t newState)
{
    statesMutex.lock();
    act_state = newState;
    statesMutex.unlock();
}

int8_t Joystick::map_js_axis_value_int8(int js_axis_value)
{
    int js_axis_span = max_js_axis_value - min_js_axis_value;
    double percentage = ((double) js_axis_value - (double) min_js_axis_value) / (double) js_axis_span;

    return std::numeric_limits<int8_t>::min() + percentage * (std::numeric_limits<int8_t>::max() - std::numeric_limits<int8_t>::min());
}

uint8_t Joystick::map_js_axis_value_uint8(int js_axis_value)
{
    int js_axis_span = max_js_axis_value - min_js_axis_value;
    double percentage = ((double) js_axis_value - (double) min_js_axis_value) / (double) js_axis_span;

    return std::numeric_limits<uint8_t>::min() + percentage * (std::numeric_limits<uint8_t>::max() - std::numeric_limits<uint8_t>::min());
}


bool Joystick::init_joystick()
{
    if (SDL_Init(SDL_INIT_JOYSTICK) == 0)
    {
        SDL_JoystickEventState(SDL_ENABLE);
        js = SDL_JoystickOpen(0);
    }

    return js != nullptr;
}

void Joystick::updateMsgOut(SDL_Event event)
{
    switch (event.type)
    {
        case SDL_JOYAXISMOTION:
        {
            if (abs(event.jaxis.value) < JOY_DEAD_CENTER_ZONE)
            {
                return;
            }

            if (R2_AXIS == event.jaxis.axis)
            {
                _msgOut.r2_axis = map_js_axis_value_uint8(event.jaxis.value);
            }
            else if (L2_AXIS == event.jaxis.axis)
            {
                _msgOut.l2_axis = map_js_axis_value_uint8(event.jaxis.value);
            }
            else if (L3_HORIZONTAL_AXIS == event.jaxis.axis)
            {
                _msgOut.l3_x_axis = map_js_axis_value_int8(event.jaxis.value);
            }
            else if (L3_VERTICAL_AXIS == event.jaxis.axis)
            {
                _msgOut.l3_y_axis = map_js_axis_value_int8(event.jaxis.value);
            }
            else if (R3_HORIZONTAL_AXIS == event.jaxis.axis)
            {
                _msgOut.r3_x_axis = map_js_axis_value_int8(event.jaxis.value);
            }
            else if (R3_VERTICAL_AXIS == event.jaxis.axis)
            {
                _msgOut.r3_y_axis = map_js_axis_value_int8(event.jaxis.value);
            }
            break;
        }
        case  SDL_JOYBUTTONUP:
        {
            if (CIRCLE_BUTTON == event.jbutton.button)
            {
                _msgOut.l2_axis = 0;
                _msgOut.r2_axis = 0;
                _msgOut.l3_x_axis = 0;
                _msgOut.l3_y_axis = 0;
                _msgOut.r3_x_axis = 0;
                _msgOut.r3_y_axis = 0;
            }

            emit btnPressed(event.jbutton.button);
            break;
        }
        case SDL_JOYHATMOTION:
        {
            switch(event.jhat.value)
            {

            case SDL_HAT_UP: break;

            case SDL_HAT_LEFT: break;

            case SDL_HAT_RIGHT: break;

            case SDL_HAT_DOWN: break;
            }

            break;
        }

        default:
            break;

    }
}

void Joystick::run()
{
    while (act_state != EXIT)
    {
        emit msgOut(_msgOut);
        switch (act_state)
        {
            case IDLE:
            {
                if (init_joystick())
                {
                    updateState(OPERATIVE);
                    emit jsConnected(true);
                }
                else
                {
                    updateState(IDLE);
                    emit jsConnected(false);
                }
            }
            break;
            case OPERATIVE:
            {
                SDL_Event event;
                if (SDL_NumJoysticks() == 0)
                {
                    updateState(IDLE);
                }
                else
                {
                    while (SDL_PollEvent(&event))
                    {
                        updateMsgOut(event);
                    }
                }

            }
            break;
            case EXIT:
            break;
            default:
            break;
        }
    }
}
