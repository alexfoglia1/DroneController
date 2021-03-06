#ifndef PROTO_H
#define PROTO_H

#include <stdint.h>
#include <QObject>

#define CTRL_TO_RADIO_CFG_ID 0
#define CTRL_TO_RADIO_CMD_ID 1
#define RADIO_TO_CTRL_ECHO_ID 252
#define RADIO_TO_CTRL_CFG_ID 253
#define RADIO_TO_CTRL_ALIVE_ID 254

#define START_MARKER 60 // '<'
#define END_MARKER   62 // '>'

#ifdef WIN32
#pragma pack(push, 1)
#endif

#ifdef WIN32
typedef struct
#else
typedef struct __attribute__((packed))
#endif
{
    uint8_t msg_id;
    uint64_t rx_pipe;
    uint64_t tx_pipe;
    uint8_t  config_ok;

} CtrlToRadioConfigMessage;

#ifdef WIN32
typedef struct
#else
typedef struct __attribute__((packed))
#endif
{
    uint8_t msg_id;
    uint8_t l2_axis;
    uint8_t r2_axis;
    int8_t l3_x_axis;
    int8_t l3_y_axis;
    int8_t r3_x_axis;
    int8_t r3_y_axis;
} CtrlToRadioCommandMessage;

#ifdef WIN32
typedef struct
#else
typedef struct __attribute__((packed))
#endif
{
  uint8_t msg_id;
  char  major_v;
  char  minor_v;
  char  stage_v;
  char  drone_alive;
} RadioToCtrlAliveMessage;


#ifdef WIN32
typedef struct
#else
typedef struct __attribute__((packed))
#endif
{
    uint8_t msg_id;
    uint64_t rx_pipe;
    uint64_t tx_pipe;
} RadioToCtrlConfigMessage;

#ifdef WIN32
typedef struct
#else
typedef struct __attribute__((packed))
#endif
{
  uint8_t msg_id;
  char     fw_major_v;
  char     fw_minor_v;
  char     fw_stage_v;
  uint8_t  motors_armed;
  uint16_t motor1_speed;
  uint16_t motor2_speed;
  uint16_t motor3_speed;
  uint16_t motor4_speed;
  uint16_t heading;
  uint16_t pitch;
  uint16_t roll;
  float baro_altitude;
  //float gnd_distance;
} DroneToRadioResponseMessage;

Q_DECLARE_METATYPE(CtrlToRadioCommandMessage)
Q_DECLARE_METATYPE(DroneToRadioResponseMessage);

#ifdef WIN32
#pragma pack(pop)
#endif

#endif //PROTO_H
