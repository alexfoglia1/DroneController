
#include <Servo.h>
#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>

#include "IMU.h"
#include "PID.h"
#include "MAINT.h"
#include "Utils.h"

#define MAJOR_VERSION '1'
#define MINOR_VERSION '0'
#define STAGE_VERSION 'B'

#define LCD_COLS 16
#define LCD_ROWS 2

#define MAX_MOTOR_SIGNAL 1500
#define MIN_MOTOR_SIGNAL 1000

#define MAX_RADIO_SIGNAL 2000
#define MIN_RADIO_SIGNAL 1000

#define MOTOR1_PIN 7
#define MOTOR2_PIN 4
#define MOTOR3_PIN 6
#define MOTOR4_PIN 5

#define CHANNEL_1_PIN 12
#define CHANNEL_2_PIN 11
#define CHANNEL_3_PIN 10
#define CHANNEL_4_PIN  9
#define CHANNEL_5_PIN  8

#define OSCILLOSCOPE_CH_1_PIN 2
#define OSCILLOSCOPE_CH_2_PIN 3

#define RED_LED_PIN  13

#define MOTORS_ARM_THRESHOLD 100 // motors start spinning at MIN_SIGNAL + x
#define CHANNEL(N)(N-1)
#define MOTOR(N)(N-1)
#define THROTTLE_CHANNEL   CHANNEL(3)
#define ROLL_CHANNEL       CHANNEL(1)
#define PITCH_CHANNEL      CHANNEL(2)
#define MOTORS_ARM_CHANNEL CHANNEL(5)

#define SAMPLING_PERIOD_S 0.025f
#define SAMPLING_PERIOD_US 25000UL
#define SECONDS_TO_MICROSECONDS 1000000UL

#define READ_COMMAND_THRESHOLD 0
#define DISPLAY_THRESHOLD 50


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
hd44780_I2Cexp lcd;

drone_attitude_t attitude;
bool lsm9ds1_found;
int count_to_command;
int count_to_disp;
uint64_t last_exec_micros;
uint64_t loop_time;
float channels[5];
float channels_dead_center_zones[5][2];
bool motors_armed;
bool motors_armed_rise;
bool motors_armed_fall;


bool pin1_state = false;
void setup(void)
{
  // HW Initialisation
  pinMode(RED_LED_PIN,  OUTPUT);

  pinMode(OSCILLOSCOPE_CH_1_PIN, OUTPUT);
  pinMode(OSCILLOSCOPE_CH_2_PIN, OUTPUT);

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

  digitalWrite(OSCILLOSCOPE_CH_1_PIN, LOW);
  digitalWrite(OSCILLOSCOPE_CH_2_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);
  
  lcd.begin(LCD_COLS, LCD_ROWS);
  lcd.setCursor(0,0);
  lcd.print("ROLL PITCH YAW");
   
  Serial.begin(115200);
  
  // SW Initialisation
  lsm9ds1_found = IMU_Init(SAMPLING_PERIOD_S);
  
  MAINT_Init(MAJOR_VERSION, MINOR_VERSION, STAGE_VERSION);
  
  attitude.data.pitch = 0;
  attitude.data.roll  = 0;
  attitude.data.yaw   = 0;

  motors_armed = false;
  motors_armed_rise = false;
  motors_armed_fall = false;

  count_to_disp = 0;
  count_to_command = 0;
  
  for (int i = CHANNEL(1); i <= CHANNEL(5); i++)
  {
    channels[i] = 0.0f;
    channels_dead_center_zones[i][0] = 0.0f;
    channels_dead_center_zones[i][1] = 0.0f;
  }

  channels_dead_center_zones[ROLL_CHANNEL][0] = 0.38f;
  channels_dead_center_zones[ROLL_CHANNEL][1] = 0.43f;
  channels_dead_center_zones[PITCH_CHANNEL][0] = 0.38f;
  channels_dead_center_zones[PITCH_CHANNEL][1] = 0.43f;

  channels[MOTORS_ARM_CHANNEL] = 0.0f;
  channels[THROTTLE_CHANNEL] = 0.0f;
  channels[PITCH_CHANNEL] = 0.4f;
  channels[ROLL_CHANNEL] = 0.0f;

  last_exec_micros = 0;
  loop_time = 0;
}

void loop(void)
{ 
  pin1_state = !pin1_state;
// --------------------------------------------- TIMING CONSTRAINTS -------------------------------------------------
  uint64_t t0_micros = micros();
  if (last_exec_micros == 0UL)
  {
    last_exec_micros = t0_micros;
  }
  else
  {
    uint64_t delta_micros = t0_micros - last_exec_micros;
    if (delta_micros < SAMPLING_PERIOD_US)
    {
      uint64_t remaining_micros = (SAMPLING_PERIOD_US - delta_micros);
      digitalWrite(OSCILLOSCOPE_CH_2_PIN, HIGH);
      delayMicroseconds(remaining_micros);
      digitalWrite(OSCILLOSCOPE_CH_2_PIN, LOW);
    }
  }
  
  digitalWrite(OSCILLOSCOPE_CH_1_PIN, pin1_state ? HIGH : LOW);
  uint64_t cur_micros = micros();
  loop_time = cur_micros - last_exec_micros;
  last_exec_micros = cur_micros;
// ----------------------------------------------------------------------------------------------------------------- 
  uint16_t motors_speed[4] = {MIN_MOTOR_SIGNAL, MIN_MOTOR_SIGNAL, MIN_MOTOR_SIGNAL, MIN_MOTOR_SIGNAL};
  if (lsm9ds1_found)
  {
// --------------------------------------------- READ CURRENT ATTITUDE --------------------------------------------- 
    float acc[3]  = {0.f, 0.f, 0.f};
    float gyro[3] = {0.f, 0.f, 0.f};
    float magn[3] = {0.f, 0.f, 0.f};

    // Read IMU and update kalman filter
    if (motors_armed_rise)
    {
      IMU_EnableMovingAVGFilter();
      MAINT_UpdateMovingAVGFilterState(1);
    }
    
    if (motors_armed_fall)
    {
      IMU_DisableMovingAVGFilter();
      MAINT_UpdateMovingAVGFilterState(0);
    }
    
    IMU_Update(acc, gyro, magn);
    MAINT_UpdateIMU(acc, gyro, magn);
    // ----------------------------------
    // Get current attitude
    IMU_CurrentAttitude(&attitude.data.roll, &attitude.data.pitch, &attitude.data.yaw);

    // Display current attitude
    if (count_to_disp == DISPLAY_THRESHOLD)
    {
      char ascii[5];
      int_to_ascii((int)attitude.data.roll, ascii, 5);
      
      lcd.setCursor(0, 1);
      lcd.print(ascii);
    }

    if (count_to_disp == 1 + DISPLAY_THRESHOLD)
    {
      char ascii[5];
      int_to_ascii((int)attitude.data.pitch, ascii, 5);

      lcd.setCursor(5, 1);
      lcd.print(ascii);
    }

    if (count_to_disp == 2 + DISPLAY_THRESHOLD)
    {
      char ascii[5];
      int_to_ascii((int)attitude.data.yaw, ascii, 5);

      lcd.setCursor(10, 1);
      lcd.print(ascii);

      count_to_disp = 0;
    }
    else
    {
      count_to_disp += 1;
    }
    
    MAINT_UpdateAHRS(attitude.vect);
// -----------------------------------------------------------------------------------------------------------------
// -------------------------------------------- READ COMMAND FROM RADIO --------------------------------------------
    if (count_to_command == READ_COMMAND_THRESHOLD)
    {
      channels[CHANNEL(1)] = normalizedPulseIn(CHANNEL_1_PIN, MIN_RADIO_SIGNAL, MAX_RADIO_SIGNAL, 16000);
    }

    if (count_to_command == 1 + READ_COMMAND_THRESHOLD)
    {
      channels[CHANNEL(2)] = normalizedPulseIn(CHANNEL_2_PIN, MIN_RADIO_SIGNAL, MAX_RADIO_SIGNAL, 16000);
    }

    if (count_to_command == 2 + READ_COMMAND_THRESHOLD)
    {
      channels[CHANNEL(3)] = normalizedPulseIn(CHANNEL_3_PIN, MIN_RADIO_SIGNAL, MAX_RADIO_SIGNAL, 16000);
    }

    if (count_to_command == 3 + READ_COMMAND_THRESHOLD)
    {
      channels[CHANNEL(5)] = normalizedPulseIn(CHANNEL_5_PIN, MIN_RADIO_SIGNAL, MAX_RADIO_SIGNAL, 16000);
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
    PID_Update(rx_angle, fake_attitude, SAMPLING_PERIOD_S);
    float* PID = PID_Get();
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
    else
    {
      digitalWrite(RED_LED_PIN, HIGH);
      lcd.setCursor(0, 0);
      lcd.print("IMU :          ");
      lcd.setCursor(0, 1);
      lcd.print("NO SUCH DEVICE");
    }
// -----------------------------------------------------------------------------------------------------------------
// -------------------------------------------- MOTORS CONTROL ----------------------------------------------------- 
  motor1.writeMicroseconds(motors_armed ? motors_speed[MOTOR(1)] : MIN_MOTOR_SIGNAL);
  motor2.writeMicroseconds(motors_armed ? motors_speed[MOTOR(2)] : MIN_MOTOR_SIGNAL);
  motor3.writeMicroseconds(motors_armed ? motors_speed[MOTOR(3)] : MIN_MOTOR_SIGNAL);
  motor4.writeMicroseconds(motors_armed ? motors_speed[MOTOR(4)] : MIN_MOTOR_SIGNAL);
// -----------------------------------------------------------------------------------------------------------------
// -------------------------------------------- MAINTENANCE  -------------------------------------------------------
  MAINT_UpdateLoopTime(loop_time);
  Serial.write((uint8_t*)MAINT_Get(), sizeof(maint_data_t));
// -----------------------------------------------------------------------------------------------------------------
}
