#ifndef PROTO_H
#define PROTO_H

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
