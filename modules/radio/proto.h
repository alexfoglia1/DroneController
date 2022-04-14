#ifndef PROTO_H
#define PROTO_H

#include <stdint.h>
#include <QObject>

#define CTRL_TO_RADIO_MSG_ID 1

typedef struct
{
    uint8_t msg_id;
    uint8_t l2_axis;
    uint8_t r2_axis;
    int8_t l3_x_axis;
    int8_t l3_y_axis;
    int8_t r3_x_axis;
    int8_t r3_y_axis;
} CtrlToRadioMsg;

Q_DECLARE_METATYPE(CtrlToRadioMsg)

#endif //PROTO_H
