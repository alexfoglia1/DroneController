#ifndef PROTO_H
#define PROTO_H

#include <stdint.h>
#include <QObject>

#define CTRL_TO_RADIO_CFG_ID 0
#define CTRL_TO_RADIO_CMD_ID 1
#define RADIO_TO_CTRL_ALIVE_ID 254
#define RADIO_TO_CTRL_ACK_ID 255

#define START_MARKER 60 // '<'
#define END_MARKER   62 // '>'


typedef struct __attribute__((packed))
{
    uint32_t msg_id;
    uint64_t rx_pipe;
    uint64_t tx_pipe;
} CtrlToRadioConfig;

typedef struct __attribute__((packed))
{
    uint32_t msg_id;
    uint8_t l2_axis;
    uint8_t r2_axis;
    int8_t l3_x_axis;
    int8_t l3_y_axis;
    int8_t r3_x_axis;
    int8_t r3_y_axis;
} CtrlToRadioCommand;


typedef struct __attribute__((packed))
{
  uint32_t msg_id;
  uint32_t msg_acked;
  uint8_t  ack_status;
} RadioToCtrlAckMessage;


typedef struct __attribute__((packed))
{
  uint32_t msg_id;
  char  major_v;
  char  minor_v;
  char  stage_v;
} RadioToCtrlAliveMessage;

Q_DECLARE_METATYPE(CtrlToRadioCommand)

#endif //PROTO_H
