#include <Servo.h>

#include "IMU.h"
#include "PID.h"

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

#define MOTOR_ARM_THRESHOLD 200 // motors start spinning at MIN_SIGNAL + 200
#define PID_START_THRESHOLD 50

typedef union
{
  struct
  {
    float roll;
    float pitch;
    float yaw;
  } data;
  
  float vect[3];
} drone_attitude_t;

Servo motor1;
Servo motor2;
Servo motor3;
Servo motor4;

drone_attitude_t attitude_km1;
drone_attitude_t attitude;
bool lsm9ds1_found;
int count_to_pid;


float minMax(float value, float min_value, float max_value)
{
    if (value > max_value) {
        value = max_value;
    } else if (value < min_value) {
        value = min_value;
    }

    return value;
}


float toRange(float value, float rangeIn, float rangeOut)
{
  return (value/rangeIn) * rangeOut;
}


void setup(void)
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  
  Serial.begin(115200);
  
  motor1.attach(MOTOR1_PIN);
  motor2.attach(MOTOR2_PIN);
  motor3.attach(MOTOR3_PIN);
  motor4.attach(MOTOR4_PIN);

  lsm9ds1_found = IMU_Init();
  
  attitude.data.pitch = 0;
  attitude.data.roll  = 0;
  attitude.data.yaw   = 0;

  attitude_km1.data.pitch = 0;
  attitude_km1.data.roll  = 0;
  attitude_km1.data.yaw   = 0;

  count_to_pid = 0;

  motor1.writeMicroseconds(MIN_SIGNAL);
  motor2.writeMicroseconds(MIN_SIGNAL);
  motor3.writeMicroseconds(MIN_SIGNAL);
  motor4.writeMicroseconds(MIN_SIGNAL);

  Serial.println("Press enter");
  while (!Serial.available());
  Serial.read();
  //Serial.println("PID_ROLL, PID_PITCH, PID_YAW");
  //delay(100);
}


void loop(void)
{
   int MOTOR1_SPEED = MIN_SIGNAL;
   int MOTOR2_SPEED = MIN_SIGNAL;
   int MOTOR3_SPEED = MIN_SIGNAL;
   int MOTOR4_SPEED = MIN_SIGNAL;

  if (lsm9ds1_found)
  {
    IMU_Update();

    IMU_CurrentAttitude(&attitude.data.roll, &attitude.data.pitch, &attitude.data.yaw);

    /** Attitude stabilized if |heading[k - 1] - heading[k]|| > 0.25 **/
    float delta_yaw_mod = fabs(attitude_km1.data.yaw - attitude.data.yaw);
    bool attitude_stabilized = (delta_yaw_mod < 0.25f);
    digitalWrite(LED_BUILTIN, attitude_stabilized ? LOW : HIGH);
    
    attitude_km1.data.roll  = attitude.data.roll;
    attitude_km1.data.pitch = attitude.data.pitch;
    attitude_km1.data.yaw   = attitude.data.yaw;
    
    /** Wait PID_START_THRESHOLD consecutive attitude_stabilized before PID loop **/
    if (count_to_pid < PID_START_THRESHOLD)
    {
       count_to_pid = attitude_stabilized ? count_to_pid + 1 : 0;
    }
    else
    {
      digitalWrite(LED_BUILTIN, LOW);
      
      float fake_command[3] = {0.0f, 0.0f, 0.0f};
      PID_Update(fake_command, attitude.vect);
      float pid[3];
      pid[ROLL]  = toRange(PID[ROLL],  100.0f, 1000.0f);
      pid[PITCH] = toRange(PID[PITCH], 100.0f, 1000.0f);
      pid[YAW]   = toRange(PID[YAW],   100.0f, 1000.0); 
      //Serial.print(PID[ROLL]);
      //Serial.print(" ");
      //Serial.print(PID[PITCH]);
      //Serial.print(" ");
      //Serial.println(PID[YAW]);
      
      MOTOR1_SPEED = minMax(MIN_SIGNAL + MOTOR_ARM_THRESHOLD + pid[ROLL] + pid[PITCH] - pid[YAW], MIN_SIGNAL + MOTOR_ARM_THRESHOLD, 2000);
      MOTOR4_SPEED = minMax(MIN_SIGNAL + MOTOR_ARM_THRESHOLD - pid[ROLL] + pid[PITCH] + pid[YAW], MIN_SIGNAL + MOTOR_ARM_THRESHOLD, 2000);
      MOTOR2_SPEED = minMax(MIN_SIGNAL + MOTOR_ARM_THRESHOLD + pid[ROLL] - pid[PITCH] + pid[YAW], MIN_SIGNAL + MOTOR_ARM_THRESHOLD, 2000);
      MOTOR3_SPEED = minMax(MIN_SIGNAL + MOTOR_ARM_THRESHOLD - pid[ROLL] - pid[PITCH] - pid[YAW], MIN_SIGNAL + MOTOR_ARM_THRESHOLD, 2000);
    }
  }

  Serial.print(MOTOR1_SPEED);
  Serial.print("\t");
  Serial.print(MOTOR4_SPEED);
  Serial.print("\t");
  Serial.print(MOTOR2_SPEED);
  Serial.print("\t");
  Serial.println(MOTOR3_SPEED);

  
  // Front left motor : M1
  motor1.writeMicroseconds(MOTOR1_SPEED);
  // Front right motor: M4
  motor4.writeMicroseconds(MOTOR4_SPEED);
  // Back left motor: M2
  motor2.writeMicroseconds(MOTOR2_SPEED);
  // Back right motor: M3
  motor3.writeMicroseconds(MOTOR3_SPEED);
}
