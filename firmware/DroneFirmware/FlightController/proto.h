#ifndef PROTO_H
#define PROTO_H

#define RADIO_TO_DRONE_MSG_ID 100
#define DRONE_TO_RADIO_MSG_ID 101

#define IMU_NO_SUCH_DEVICE  0x01
#define BARO_NO_SUCH_DEVICE 0x10
#define NO_ERRORS           0x00

typedef struct __attribute__((packed))
{
    unsigned char msg_id;
    unsigned char l2_axis;
    unsigned char r2_axis;
    char l3_x_axis;
    char l3_y_axis;
    char r3_x_axis;
    char r3_y_axis;
} CtrlToRadioCommandMessage;

typedef struct __attribute__((packed))
{
  unsigned char  msg_id;
  char     fw_major_v;
  char     fw_minor_v;
  char     fw_stage_v;
  unsigned char  motors_armed;
  unsigned short motor1_speed;
  unsigned short motor2_speed;
  unsigned short motor3_speed;
  unsigned short motor4_speed;
  unsigned short heading;
  unsigned short pitch;
  unsigned short roll;
  float baro_altitude;
  //float gnd_distance;
} DroneToRadioResponseMessage;


#endif //PROTO_H
