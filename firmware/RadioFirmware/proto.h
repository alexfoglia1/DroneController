#ifndef PROTO_H
#define PROTO_H

#define CTRL_TO_RADIO_CFG_ID 0
#define CTRL_TO_RADIO_CMD_ID 1

#define RADIO_TO_CTRL_ECHO_ID 252
#define RADIO_TO_CTRL_CFG_ID 253
#define RADIO_TO_CTRL_ALIVE_ID 254

#define RADIO_TO_DRONE_MSG_ID 100
#define DRONE_TO_RADIO_MSG_ID 101

uint8_t startMarker = 60;
uint8_t endMarker = 62;

typedef struct __attribute__((packed))
{
    uint32_t msg_id;
    uint64_t rx_pipe;
    uint64_t tx_pipe;
    uint8_t  config_ok;
} CtrlToRadioConfigMessage;

typedef struct __attribute__((packed))
{
    uint32_t msg_id;
    uint8_t l2_axis;
    uint8_t r2_axis;
    int8_t l3_x_axis;
    int8_t l3_y_axis;
    int8_t r3_x_axis;
    int8_t r3_y_axis;
} CtrlToRadioCommandMessage;

typedef struct __attribute__((packed))
{
  uint32_t msg_id;
  char  major_v;
  char  minor_v;
  char  stage_v;
} RadioToCtrlAliveMessage;

typedef struct __attribute__((packed))
{
    uint32_t msg_id;
    uint64_t rx_pipe;
    uint64_t tx_pipe;
} RadioToCtrlConfigMessage;

typedef struct __attribute__((packed))
{
  uint32_t msg_id;
  CtrlToRadioCommandMessage echoed;
  uint16_t motor1_speed;
  uint16_t motor2_speed;
  uint16_t motor3_speed;
  uint16_t motor4_speed;
  uint16_t heading;
  uint16_t pitch;
  uint16_t roll;
  uint16_t baro_altitude;
} DroneToRadioResponseMessage;



#endif //PROTO_H
