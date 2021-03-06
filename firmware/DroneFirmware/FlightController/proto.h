#ifndef PROTO_H
#define PROTO_H

#define RADIO_TO_DRONE_MSG_ID 100
#define DRONE_TO_RADIO_MSG_ID 101
#define BLE_TO_UNO_MSG_ID     102
#define BLE_TO_UNO_EMSG_ID    103

#define IMU_NO_SUCH_DEVICE  0x01
#define BARO_NO_SUCH_DEVICE 0x10
#define NO_ERRORS           0x00

uint8_t startMarker = 60;
uint8_t endMarker = 62;

typedef struct __attribute__((packed))
{
    uint8_t msg_id;
    uint8_t l2_axis;
    uint8_t r2_axis;
    int8_t l3_x_axis;
    int8_t l3_y_axis;
    int8_t r3_x_axis;
    int8_t r3_y_axis;
} CtrlToRadioCommandMessage;

typedef struct __attribute__((packed))
{
  uint8_t  msg_id;
  byte     fw_major_v;
  byte     fw_minor_v;
  byte     fw_stage_v;
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

typedef struct __attribute__((packed))
{
  uint8_t msg_id;
  float pitch;
  float roll;
  float yaw;
  float baro_altitude;
} BleToUnoMessage;

typedef struct __attribute__((packed))
{
  uint8_t msg_id;
  uint8_t error_code;
} BleToUnoErrorMessage;


#endif //PROTO_H
