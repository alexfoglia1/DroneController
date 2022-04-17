#define RADIO_TO_DRONE_MSG_ID 100
#define DRONE_TO_RADIO_MSG_ID 101

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
  CtrlToRadioCommandMessage echoed;
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
  uint16_t baro_altitude;
} DroneToRadioResponseMessage;
