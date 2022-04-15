#ifndef PROTO_H
#define PROTO_H

#define CTRL_TO_RADIO_CFG_ID 0
#define CTRL_TO_RADIO_CMD_ID 1

uint8_t startMarker = 60;
uint8_t endMarker = 62;

typedef struct
{
    uint32_t msg_id;
    uint64_t rx_pipe;
    uint64_t tx_pipe;
} CtrlToRadioConfig;

typedef struct
{
    uint32_t msg_id;
    uint8_t l2_axis;
    uint8_t r2_axis;
    int8_t l3_x_axis;
    int8_t l3_y_axis;
    int8_t r3_x_axis;
    int8_t r3_y_axis;
} CtrlToRadioCommand;

#endif //PROTO_H
