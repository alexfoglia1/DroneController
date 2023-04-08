
#include <Servo.h>

#include "IMU.h"
#include "PID.h"
#include "MAINT.h"
#include "Utils.h"

#define MAJOR_VERSION '1'
#define MINOR_VERSION '0'
#define STAGE_VERSION 'B'

#define MAX_MOTOR_SIGNAL 1500
#define MIN_MOTOR_SIGNAL 1000

#define MAX_RADIO_SIGNAL 2000
#define MIN_RADIO_SIGNAL 1000

#define MOTOR1_PIN 4
#define MOTOR2_PIN 7
#define MOTOR3_PIN 6
#define MOTOR4_PIN 5

#define CHANNEL_1_PIN 12
#define CHANNEL_2_PIN 11
#define CHANNEL_3_PIN 10
#define CHANNEL_4_PIN  9
#define CHANNEL_5_PIN  8

#define RED_LED_PIN  13

#define MOTORS_ARM_THRESHOLD 100 // motors start spinning at MIN_SIGNAL + x
#define CHANNEL(N)(N-1)
#define MOTOR(N)(N-1)
#define THROTTLE_CHANNEL   CHANNEL(3)
#define ROLL_CHANNEL       CHANNEL(1)
#define PITCH_CHANNEL      CHANNEL(2)
#define MOTORS_ARM_CHANNEL CHANNEL(5)

#define READ_COMMAND_THRESHOLD 0


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
bool lsm9ds1_found;
int count_to_command;
float channels[5];
float channels_dead_center_zones[5][2];
bool motors_armed;
bool motors_armed_rise;
bool motors_armed_fall;

void setup(void)
{
  // HW Initialisation
  pinMode(RED_LED_PIN,  OUTPUT);

  pinMode(CHANNEL_1_PIN, INPUT);
  pinMode(CHANNEL_2_PIN, INPUT);
  pinMode(CHANNEL_3_PIN, INPUT);
  pinMode(CHANNEL_4_PIN, INPUT);
  pinMode(CHANNEL_5_PIN, INPUT);
  
  motor1.attach(MOTOR1_PIN);
  motor2.attach(MOTOR2_PIN);
  motor3.attach(MOTOR3_PIN);
  motor4.attach(MOTOR4_PIN);

  motor1.writeMicroseconds(MIN_MOTOR_SIGNAL);
  motor2.writeMicroseconds(MIN_MOTOR_SIGNAL);
  motor3.writeMicroseconds(MIN_MOTOR_SIGNAL);
  motor4.writeMicroseconds(MIN_MOTOR_SIGNAL);

  digitalWrite(RED_LED_PIN, LOW);

  // SW Initialisation
  Serial.begin(115200);
  lsm9ds1_found = IMU_Init();
  
  MAINT_Init(MAJOR_VERSION, MINOR_VERSION, STAGE_VERSION);
  
  attitude.data.pitch = 0;
  attitude.data.roll  = 0;
  attitude.data.yaw   = 0;

  motors_armed = false;
  motors_armed_rise = false;
  motors_armed_fall = false;

  count_to_command = 0;
  
  for (int i = CHANNEL(1); i <= CHANNEL(5); i++)
  {
    channels[i] = 0.5f;
    channels_dead_center_zones[i][0] = 0.0f;
    channels_dead_center_zones[i][1] = 0.0f;
  }

  channels_dead_center_zones[ROLL_CHANNEL][0] = 0.38f;
  channels_dead_center_zones[ROLL_CHANNEL][1] = 0.43f;
  channels_dead_center_zones[PITCH_CHANNEL][0] = 0.38f;
  channels_dead_center_zones[PITCH_CHANNEL][1] = 0.43f;
}


void loop(void)
{ 
  uint16_t motors_speed[4] = {MIN_MOTOR_SIGNAL, MIN_MOTOR_SIGNAL, MIN_MOTOR_SIGNAL, MIN_MOTOR_SIGNAL};

  if (lsm9ds1_found)
  {
// --------------------------------------------- READ CURRENT ATTITUDE --------------------------------------------- 
    float acc[3]  = {0.f, 0.f, 0.f};
    float gyro[3] = {0.f, 0.f, 0.f};
    float magn[3] = {0.f, 0.f, 0.f};
    uint64_t dt = 0;

    // Read IMU and update kalman filter
    if (motors_armed_rise)
    {
      IMU_EnableFilters();
      MAINT_UpdateButterworthFilterState(1);
    }
    
    if (motors_armed_fall)
    {
      IMU_DisableFilters();
      MAINT_UpdateButterworthFilterState(0);
    }
    
    IMU_Update(acc, gyro, magn, &dt);
    MAINT_UpdateIMU(acc, gyro, magn);
    // ----------------------------------
    // Get current attitude
    IMU_CurrentAttitude(&attitude.data.roll, &attitude.data.pitch, &attitude.data.yaw);
    MAINT_UpdateKF(dt, attitude.vect);

// -----------------------------------------------------------------------------------------------------------------
// -------------------------------------------- READ COMMAND FROM RADIO --------------------------------------------
    if (count_to_command == READ_COMMAND_THRESHOLD)
    {
      channels[CHANNEL(1)] = normalizedPulseIn(CHANNEL_1_PIN, MIN_RADIO_SIGNAL, MAX_RADIO_SIGNAL);
    }

    if (count_to_command == 1 + READ_COMMAND_THRESHOLD)
    {
      channels[CHANNEL(2)] = normalizedPulseIn(CHANNEL_2_PIN, MIN_RADIO_SIGNAL, MAX_RADIO_SIGNAL);
    }

    if (count_to_command == 2 + READ_COMMAND_THRESHOLD)
    {
      channels[CHANNEL(3)] = normalizedPulseIn(CHANNEL_3_PIN, MIN_RADIO_SIGNAL, MAX_RADIO_SIGNAL);
    }

    if (count_to_command == 3 + READ_COMMAND_THRESHOLD)
    {
      channels[CHANNEL(5)] = normalizedPulseIn(CHANNEL_5_PIN, MIN_RADIO_SIGNAL, MAX_RADIO_SIGNAL);
      count_to_command = 0;
    }
    else
    {
      count_to_command += 1;
    }


    bool motors_armed_condition = (channels[MOTORS_ARM_CHANNEL] > 0.5f);
    if (motors_armed == false && motors_armed_condition == true)
    {
      motors_armed_rise = true;
    }
    else
    {
      motors_armed_rise = false;
    }

    if (motors_armed == true && motors_armed_condition == false)
    {
      motors_armed_fall = true;
    }
    else
    {
      motors_armed_fall = false;
    }

    motors_armed = motors_armed_condition;
                                             
    float roll =  (channels[ROLL_CHANNEL]  >= channels_dead_center_zones[ROLL_CHANNEL][0]  && channels[ROLL_CHANNEL]  <= channels_dead_center_zones[ROLL_CHANNEL][1])  ? 0.0f : toRange(channels[ROLL_CHANNEL],  0.0f, 0.85f, -2.0f, 2.0f);
    float pitch = (channels[PITCH_CHANNEL] >= channels_dead_center_zones[PITCH_CHANNEL][0] && channels[PITCH_CHANNEL] <= channels_dead_center_zones[PITCH_CHANNEL][1]) ? 0.0f : toRange(channels[PITCH_CHANNEL], 0.0f, 0.85f, -2.0f, 2.0f);
    
    float fake_attitude[3] = {0.0f, 0.0f, 0.0f};
    float rx_angle[3] = {roll, pitch, 0.0f};
    MAINT_UpdateCMD(channels[THROTTLE_CHANNEL], rx_angle);

// -----------------------------------------------------------------------------------------------------------------
// -------------------------------------------- PID UPDATE --------------------------------------------------------- 
    PID_Update(rx_angle, fake_attitude, dt*1e-6);
    MAINT_UpdatePID(PID);

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
       motors_speed[MOTOR(1)] = minMax(
                                       toRange(channels[THROTTLE_CHANNEL], 0.0f, 1.0f, MIN_MOTOR_SIGNAL + MOTORS_ARM_THRESHOLD, MAX_MOTOR_SIGNAL) + PID[ROLL] - PID[PITCH],
                                       MIN_MOTOR_SIGNAL + MOTORS_ARM_THRESHOLD,
                                       MAX_MOTOR_SIGNAL);
       motors_speed[MOTOR(2)] = minMax(
                                       toRange(channels[THROTTLE_CHANNEL], 0.0f, 1.0f, MIN_MOTOR_SIGNAL + MOTORS_ARM_THRESHOLD, MAX_MOTOR_SIGNAL) - PID[ROLL] - PID[PITCH],
                                       MIN_MOTOR_SIGNAL + MOTORS_ARM_THRESHOLD,
                                       MAX_MOTOR_SIGNAL);
       motors_speed[MOTOR(3)] = minMax(
                                      toRange(channels[THROTTLE_CHANNEL], 0.0f, 1.0f, MIN_MOTOR_SIGNAL + MOTORS_ARM_THRESHOLD, MAX_MOTOR_SIGNAL) - PID[ROLL] + PID[PITCH],
                                      MIN_MOTOR_SIGNAL + MOTORS_ARM_THRESHOLD,
                                      MAX_MOTOR_SIGNAL);
       motors_speed[MOTOR(4)] = minMax(
                                      toRange(channels[THROTTLE_CHANNEL], 0.0f, 1.0f, MIN_MOTOR_SIGNAL + MOTORS_ARM_THRESHOLD, MAX_MOTOR_SIGNAL) + PID[ROLL] + PID[PITCH],
                                      MIN_MOTOR_SIGNAL + MOTORS_ARM_THRESHOLD,
                                      MAX_MOTOR_SIGNAL);
       
       MAINT_UpdateMOTORS(motors_armed, motors_speed);
    }

// -----------------------------------------------------------------------------------------------------------------
// -------------------------------------------- MOTORS CONTROL ----------------------------------------------------- 
  motor1.writeMicroseconds(motors_armed ? motors_speed[MOTOR(1)] : MIN_MOTOR_SIGNAL);
  motor2.writeMicroseconds(motors_armed ? motors_speed[MOTOR(2)] : MIN_MOTOR_SIGNAL);
  motor3.writeMicroseconds(motors_armed ? motors_speed[MOTOR(3)] : MIN_MOTOR_SIGNAL);
  motor4.writeMicroseconds(motors_armed ? motors_speed[MOTOR(4)] : MIN_MOTOR_SIGNAL);
// -----------------------------------------------------------------------------------------------------------------
// -------------------------------------------- MAINTENANCE  ------------------------------------------------------- 

  uint8_t* p_maint_data = reinterpret_cast<uint8_t*>(MAINT_Get());
  Serial.write(p_maint_data, sizeof(maint_data_t));
// -----------------------------------------------------------------------------------------------------------------
}
