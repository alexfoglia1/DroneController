
#include <Servo.h>

#include "IMU.h"
#include "PID.h"
#include "MAINT.h"
#include "Utils.h"

#define MAJOR_VERSION '1'
#define MINOR_VERSION '0'
#define STAGE_VERSION 'B'

#define MAX_SIGNAL 2000
#define MIN_SIGNAL 1000

#define MOTOR1_PIN 8
#define MOTOR2_PIN 7
#define MOTOR3_PIN 6
#define MOTOR4_PIN 5

#define CHANNEL_1_PIN 13
#define CHANNEL_2_PIN 12
#define CHANNEL_3_PIN 11
#define CHANNEL_4_PIN 10
#define CHANNEL_5_PIN  9

#define YELLOW_LED_PIN A0
#define GREEN_LED_PIN  A1

#define PROBE_PIN 2

#define MOTORS_ARM_THRESHOLD 80 // motors start spinning at MIN_SIGNAL + x
#define CHANNEL(N)(N-1)
#define MOTOR(N)(N-1)
#define THROTTLE_CHANNEL   CHANNEL(3)
#define ROLL_CHANNEL       CHANNEL(1)
#define MOTORS_ARM_CHANNEL CHANNEL(5)

#define ROLL_VARIANCE_THRESHOLD  0.04f 
#define PITCH_VARIANCE_THRESHOLD 0.04f
#define VARIANCE_CONVERGENCE_THRESHOLD 50


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


drone_attitude_t attitude;
drone_attitude_t variance;
bool lsm9ds1_found;
bool attitude_converges;
int count_to_variance;

void setup(void)
{
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  
  pinMode(CHANNEL_1_PIN, INPUT);
  pinMode(CHANNEL_2_PIN, INPUT);
  pinMode(CHANNEL_3_PIN, INPUT);
  pinMode(CHANNEL_4_PIN, INPUT);
  pinMode(CHANNEL_5_PIN, INPUT);
  
  Serial.begin(115200);
  
  motor1.attach(MOTOR1_PIN);
  motor2.attach(MOTOR2_PIN);
  motor3.attach(MOTOR3_PIN);
  motor4.attach(MOTOR4_PIN);

  lsm9ds1_found = IMU_Init();
  IMU_UpdateKFBeta(0.05f);
  attitude_converges = false;
  
  MAINT_Init(MAJOR_VERSION, MINOR_VERSION, STAGE_VERSION);
  
  attitude.data.pitch = 0;
  attitude.data.roll  = 0;
  attitude.data.yaw   = 0;

  variance.data.pitch = 0;
  variance.data.roll = 0;
  variance.data.yaw = 0;

  count_to_variance = 0;

  motor1.writeMicroseconds(MIN_SIGNAL);
  motor2.writeMicroseconds(MIN_SIGNAL);
  motor3.writeMicroseconds(MIN_SIGNAL);
  motor4.writeMicroseconds(MIN_SIGNAL);

  analogWrite(YELLOW_LED_PIN, 255);
  analogWrite(GREEN_LED_PIN, 0);
  Serial.println("ROLL PITCH CONV");
}


void loop(void)
{ 
  uint16_t motors_speed[4] = {MIN_SIGNAL, MIN_SIGNAL, MIN_SIGNAL, MIN_SIGNAL};
  float channels[5] = {0.f, 0.f, 0.f, 0.f, 0.f};
  bool motors_armed = false;
  
  if (lsm9ds1_found)
  {
// --------------------------------------------- READ CURRENT ATTITUDE --------------------------------------------- 
    float acc[3]  = {0.f, 0.f, 0.f};
    float gyro[3] = {0.f, 0.f, 0.f};
    float magn[3] = {0.f, 0.f, 0.f};
    float t = 0;
    float dt = 0;
    IMU_Update(acc, gyro, magn, &t, &dt);
    MAINT_UpdateIMU(acc, gyro, magn, t, dt);
    IMU_CurrentAttitude(&attitude.data.roll, &attitude.data.pitch, &attitude.data.yaw);
    IMU_CurrentVariance(&variance.data.roll, &variance.data.pitch, &variance.data.yaw);
    MAINT_UpdateKF(attitude.vect);

    Serial.print(attitude.data.roll);
    Serial.print(" ");
    Serial.print(attitude.data.pitch);
    Serial.print(" ");

    if (!attitude_converges &&
        variance.data.roll  < ROLL_VARIANCE_THRESHOLD &&
        variance.data.pitch < PITCH_VARIANCE_THRESHOLD)
    {
      count_to_variance += 1;
      if (count_to_variance == VARIANCE_CONVERGENCE_THRESHOLD)
      {
        analogWrite(GREEN_LED_PIN, 255);
        analogWrite(YELLOW_LED_PIN, 0);
        attitude_converges = true; 
      }
    }
    else
    {
      count_to_variance = 0;
    }

    if (attitude_converges) Serial.println("10"); else Serial.println("0");

    
    //Serial.print(" ");
    //Serial.println(attitude.data.yaw);

    //Serial.println("**************************** UPDATE IMU *********************************");
    //Serial.print("acc.x(");
    //Serial.print(acc[X]);
    //Serial.print(") acc.y(");
    //Serial.print(acc[Y]);
    //Serial.print(") acc.z(");
    //Serial.print(acc[Z]);
    //Serial.println(")");
    //Serial.print("gyro.x(");
    //Serial.print(gyro[X]);
    //Serial.print(") gyro.y(");
    //Serial.print(gyro[Y]);
    //Serial.print(") gyro.z(");
    //Serial.print(gyro[Z]);
    //Serial.println(")");
    //Serial.print("magn.x(");
    //Serial.print(magn[X]);
    //Serial.print(") magn.y(");
    //Serial.print(magn[Y]);
    //Serial.print(") magn.z(");
    //Serial.print(magn[Z]);
    //Serial.println(")");
    //Serial.print("Roll(");
    //Serial.print(attitude.data.roll);
    //Serial.print(") Pitch(");
    //Serial.print(attitude.data.pitch);
    //Serial.print(") Yaw(");
    //Serial.print(attitude.data.yaw);
    //Serial.println(")");
    //Serial.println("*************************************************************************");
// -----------------------------------------------------------------------------------------------------------------
// -------------------------------------------- READ COMMAND FROM RADIO -------------------------------------------- 
    channels[CHANNEL(1)] = normalizedPulseIn(CHANNEL_1_PIN, MIN_SIGNAL, MAX_SIGNAL);
    channels[CHANNEL(2)] = normalizedPulseIn(CHANNEL_2_PIN, MIN_SIGNAL, MAX_SIGNAL);
    channels[CHANNEL(3)] = normalizedPulseIn(CHANNEL_3_PIN, MIN_SIGNAL, MAX_SIGNAL);
    channels[CHANNEL(4)] = normalizedPulseIn(CHANNEL_4_PIN, MIN_SIGNAL, MAX_SIGNAL);
    channels[CHANNEL(5)] = normalizedPulseIn(CHANNEL_5_PIN, MIN_SIGNAL, MAX_SIGNAL);

    motors_armed = channels[MOTORS_ARM_CHANNEL] > 0.5f;
    //Serial.println("**************************** UPDATE COMMAND ****************************");
    //Serial.print("Channel throttle(");
    //Serial.print(channels[THROTTLE_CHANNEL]);
    //Serial.print(") Channel roll(");
    //Serial.print(channels[ROLL_CHANNEL]);
    //Serial.println(")");

                                                 // roll channel is 0.37 when stick is in rest position on FS-I6
    float right_bias = (channels[ROLL_CHANNEL] - 0.37f)/4.0f; // bias: how much drone shall roll toward right or left
    float left_bias  = -right_bias;

    //Serial.print("Bias: L(");
    //Serial.print(left_bias);
    //Serial.print(") R(");
    //Serial.print(right_bias);
    //Serial.println(")");
// -----------------------------------------------------------------------------------------------------------------
// -------------------------------------------- PID UPDATE --------------------------------------------------------- 
   //if (attitude_converges)
   //{
      //float fake_command[3] = {0.0f, 0.0f, 0.0f};
      //PID_Update(fake_command, attitude.vect);
   //}
// -----------------------------------------------------------------------------------------------------------------
// -------------------------------------------- APPLY SPEED CORRECTION --------------------------------------------- 
      //[M1]     [M2]    (X)^
      //    \   /           |
      //     \ /            | 
      //      X            (Z)- - ->(Y)
      //     / \
      //    /   \
      //[M4]     [M3]
      
      // motors_speed[0] = M1 speed = TOP LEFT
      // motors_speed[1] = M2 speed = TOP RIGHT
      // motors_speed[2] = M3 speed = BOTTOM RIGHT
      // motors_speed[3] = M4 speed = BOTTOM LEFT

      // If I want to roll LEFT  I shall give more thrust to RIGHT motors M2,M3
      // If I want to roll RIGHT I shall give more thrust to LEFT  motors M1,M4

      // If I want to PITCH DOWN I shall give more thrust to BACK motors  M4,M3
      // If I want to PITCH UP   I shall give more thrust to FRONT motors M1,M2
      // TODO PITCH
      
      int   left_thrust  = toRange(channels[THROTTLE_CHANNEL] + right_bias, 0.0f, 1.0f, MIN_SIGNAL + MOTORS_ARM_THRESHOLD, MAX_SIGNAL);
      int   right_thrust = toRange(channels[THROTTLE_CHANNEL] + left_bias, 0.0f, 1.0f, MIN_SIGNAL + MOTORS_ARM_THRESHOLD, MAX_SIGNAL);

      motors_speed[MOTOR(1)] = left_thrust;
      motors_speed[MOTOR(4)] = motors_speed[MOTOR(1)];

      motors_speed[MOTOR(2)] = right_thrust;
      motors_speed[MOTOR(3)] = motors_speed[MOTOR(1)];
      
      ////Serial.print(" ");
      ////Serial.println(PID[ROLL]);
      
    }


// -----------------------------------------------------------------------------------------------------------------
// -------------------------------------------- MOTORS CONTROL ----------------------------------------------------- 
  //Serial.print("Motors speed: L(");
  //Serial.print(motors_speed[MOTOR(1)]);
  //Serial.print(") R(");
  //Serial.print(motors_speed[MOTOR(2)]);
  //Serial.println(")");
  //Serial.println("*************************************************************************");
  //Serial.println("");
  motor1.writeMicroseconds(motors_armed ? motors_speed[MOTOR(1)] : MIN_SIGNAL);
  motor2.writeMicroseconds(motors_armed ? motors_speed[MOTOR(2)] : MIN_SIGNAL);
  motor3.writeMicroseconds(motors_armed ? motors_speed[MOTOR(3)] : MIN_SIGNAL);
  motor4.writeMicroseconds(motors_armed ? motors_speed[MOTOR(4)] : MIN_SIGNAL);
// -----------------------------------------------------------------------------------------------------------------
// -------------------------------------------- MAINTENANCE  ------------------------------------------------------- 
  //MAINT_UpdateMOTORS(channels, motors_armed ? 0x01: 0x00, motors_speed);
  //uint8_t* p_maint_data = reinterpret_cast<uint8_t*>(MAINT_Get());
  //Serial.write(p_maint_data, sizeof(maint_data_t));
// -----------------------------------------------------------------------------------------------------------------
}
