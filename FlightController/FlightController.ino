#include <Servo.h>

#include "IMU.h"

#define APP_NAME "Arduino Flight Controller"
#define MAJOR_VERSION '1'
#define MINOR_VERSION '0'
#define STAGE_VERSION 'B'

#define MAX_SIGNAL 2000
#define MIN_SIGNAL 1000

#define MOTOR1_PIN 8
#define MOTOR2_PIN 7
#define MOTOR3_PIN 6
#define MOTOR4_PIN 5

typedef struct
{
  float roll;
  float pitch;
  float yaw;
} drone_attitude_t;

Servo motor1;
Servo motor2;
Servo motor3;
Servo motor4;

drone_attitude_t attitude_km1;
drone_attitude_t attitude;
bool lsm9ds1_found;


void setup(void)
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  
  Serial.begin(115200);
  
  motor1.attach(MOTOR1_PIN);
  motor2.attach(MOTOR2_PIN);
  motor3.attach(MOTOR3_PIN);
  motor4.attach(MOTOR4_PIN);

  lsm9ds1_found = IMU_Init();
  
  attitude.pitch = 0;
  attitude.roll  = 0;
  attitude.yaw   = 0;

  attitude_km1.pitch = 0;
  attitude_km1.roll  = 0;
  attitude_km1.yaw   = 0;
}


void loop(void)
{
  if (lsm9ds1_found)
  {
    IMU_Update();

    IMU_CurrentAttitude(&attitude.roll, &attitude.pitch, &attitude.yaw);

    /** LED_BUILTIN HIGH if ||attitude[k - 1] - attitude[k]|| > 1 **/
    float delta_att_mod = sqrt((attitude_km1.roll  - attitude.roll)  * (attitude_km1.roll  - attitude.roll)  +
                               (attitude_km1.pitch - attitude.pitch) * (attitude_km1.pitch - attitude.pitch) +
                               (attitude_km1.yaw   - attitude.yaw)   * (attitude_km1.yaw   - attitude.yaw));
    digitalWrite(LED_BUILTIN, delta_att_mod > 0.1f ? HIGH : LOW);

    attitude_km1.roll  = attitude.roll;
    attitude_km1.pitch = attitude.pitch;
    attitude_km1.yaw   = attitude.yaw;
  }

  // TODO: READ COMMAND + PID
  
  int DELAY_M1 = MIN_SIGNAL;
  int DELAY_M2 = MIN_SIGNAL;
  int DELAY_M3 = MIN_SIGNAL;
  int DELAY_M4 = MIN_SIGNAL;
  
  motor1.writeMicroseconds(DELAY_M1);
  motor2.writeMicroseconds(DELAY_M2);
  motor3.writeMicroseconds(DELAY_M3);
  motor4.writeMicroseconds(DELAY_M4);
}
