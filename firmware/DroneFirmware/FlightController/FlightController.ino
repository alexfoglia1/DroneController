#include "nRF24L01.h"
#include "RF24.h"
#include "SPI.h"
#include "proto.h"
#include "UltraSonic.h"

#include <Servo.h>

#define MAX_SIGNAL 2000
#define MIN_SIGNAL 1000
#define ARM_THRESHOLD_1 200
#define ARM_THRESHOLD_2 200
#define ARM_THRESHOLD_3 200
#define ARM_THRESHOLD_4 200
#define MOTOR_PIN1 8
#define MOTOR_PIN2 7
#define MOTOR_PIN3 6
#define MOTOR_PIN4 5

#define APP_NAME "Arduino Flight Controller"
#define MAJOR_VERSION '0'
#define MINOR_VERSION '1'
#define STAGE_VERSION 'B'

Servo motor1;
Servo motor2;
Servo motor3;
Servo motor4;
RF24 radio(9,10);
UltraSonic ultraSonic(2,3);

CtrlToRadioCommandMessage commandMsg;
DroneToRadioResponseMessage responseMsg;

const uint64_t rx_pipe = 0xE6E6E6E6E6E6;
const uint64_t tx_pipe = 0x6E6E6E6E6E6E;
uint8_t motorsArmed = false;
boolean armSwitched = false;

void setup(void)
{
  Serial.begin(9600);
  printFullName();
  clearMessages();
    
  motor1.attach(MOTOR_PIN1);
  motor2.attach(MOTOR_PIN2);
  motor3.attach(MOTOR_PIN3);
  motor4.attach(MOTOR_PIN4);
  
  radio.begin();
  radio.openReadingPipe(1, rx_pipe);
  radio.enableAckPayload();
  radio.startListening();
  radio.writeAckPayload(1, &responseMsg, sizeof(DroneToRadioResponseMessage));
}

const int MAX_TIMEOUT = 50;
int count = 0;

void loop(void)
{
  float gndDistance = ultraSonic.distance();
  int DELAY_M1 = MIN_SIGNAL;
  int DELAY_M2 = MIN_SIGNAL;
  int DELAY_M3 = MIN_SIGNAL;
  int DELAY_M4 = MIN_SIGNAL;

  boolean radioAvailable = false;
  if (!radio.available())
  {
    count += 1;
    radioAvailable = false;
  }
  else
  {
    radioAvailable = true;
    count = 0;
    radio.read((char*)&commandMsg, sizeof(CtrlToRadioCommandMessage));
  }
  if (true)
  {
    if (count == MAX_TIMEOUT)
    {
      count = 0;
      commandMsg.r3_y_axis = 1;
      
    }
    
    if (RADIO_TO_DRONE_MSG_ID == commandMsg.msg_id)
    {
      if (0xFF == commandMsg.r2_axis && !armSwitched)
      {
        motorsArmed = !motorsArmed;
        Serial.print("Motor armed switch: ");
        Serial.println(motorsArmed);
        armSwitched = true;
      }
      else if (0xFF == commandMsg.r2_axis && armSwitched)
      {
        /** Ignore this event **/
      }
      else
      {
        armSwitched = false;
      }

      int appDelayM1 = 0;
      int appDelayM2 = 0;
      int appDelayM3 = 0;
      int appDelayM4 = 0;

      int verticalSpeed_2 = 
                          /** Greater than zero (toward down on js): set to min_signal + arm threshold **/
                          commandMsg.r3_y_axis > 0 ? MIN_SIGNAL + ARM_THRESHOLD_2 :
                          /** Less than zero (toward up on js): map 128 values to [1100 - 2000] **/
                          MIN_SIGNAL + ARM_THRESHOLD_2 + (MAX_SIGNAL - MIN_SIGNAL - ARM_THRESHOLD_2) * ((-1 * commandMsg.r3_y_axis)/128.f);

      int verticalSpeed_4 = /** Greater than zero (toward down on js): set to min_signal + arm threshold **/
                          commandMsg.r3_y_axis > 0 ? MIN_SIGNAL + ARM_THRESHOLD_4 :
                          /** Less than zero (toward up on js): map 128 values to [1100 - 2000] **/
                          MIN_SIGNAL + ARM_THRESHOLD_4 + (MAX_SIGNAL - MIN_SIGNAL - ARM_THRESHOLD_4) * ((-1 * commandMsg.r3_y_axis)/128.f);
                            /** Keep motors armed for the moment **/
      int verticalSpeed_1 = verticalSpeed_2;
      int verticalSpeed_3 = verticalSpeed_4;

      if (motorsArmed)
      {
        DELAY_M2 = verticalSpeed_2;
        DELAY_M1 = verticalSpeed_1;
        DELAY_M4 = verticalSpeed_4;
        DELAY_M3 = verticalSpeed_3;
      }
      else
      {
        DELAY_M1 = MIN_SIGNAL;
        DELAY_M2 = MIN_SIGNAL;
        DELAY_M3 = MIN_SIGNAL;
        DELAY_M4 = MIN_SIGNAL;
      }
    }
    
    /** Build and send response **/
    responseMsg.motors_armed = motorsArmed ? 0x01 : 0x00;
    responseMsg.motor1_speed = DELAY_M1;
    responseMsg.motor2_speed = DELAY_M2;
    responseMsg.motor3_speed = DELAY_M3;
    responseMsg.motor4_speed = DELAY_M4;

    /** Todo: read from BLE Sense sensors state **/
    responseMsg.heading = 0;
    responseMsg.roll = 0;
    responseMsg.pitch = 0;
    responseMsg.baro_altitude = 0;
    responseMsg.gnd_distance = gndDistance;
    
    radio.writeAckPayload(1, &responseMsg, sizeof(DroneToRadioResponseMessage));
  }

  motor1.writeMicroseconds(DELAY_M1);
  motor2.writeMicroseconds(DELAY_M2);
  motor3.writeMicroseconds(DELAY_M3);
  motor4.writeMicroseconds(DELAY_M4);
       
  float SPEED_M1 = (DELAY_M1 - 1000) / 10;
  float SPEED_M2 = (DELAY_M2 - 1000) / 10;
  float SPEED_M3 = (DELAY_M3 - 1000) / 10;
  float SPEED_M4 = (DELAY_M4 - 1000) / 10;
#if 0
  if (radioAvailable)
  {
    Serial.print("Motor speed:");
    Serial.print("  "); Serial.print(SPEED_M1); Serial.print("%");
    Serial.print("  "); Serial.print(SPEED_M2); Serial.print("%");
    Serial.print("  "); Serial.print(SPEED_M3); Serial.print("%");
    Serial.print("  "); Serial.print(SPEED_M4); Serial.println("%");
  }
  else
  {
    Serial.println("Radio unavailable");
  }
  #endif
  Serial.println(gndDistance);
}

void printFullName()
{
  Serial.print(APP_NAME);
  Serial.print(" ");
  Serial.print(MAJOR_VERSION);
  Serial.print(".");
  Serial.print(MINOR_VERSION);
  Serial.print("-");
  Serial.println(STAGE_VERSION);
}

float f_min(float f1, float f2)
{
  return f1 <= f2 ? f1 : f2;
}

void clearMessages()
{
  commandMsg.msg_id = 0;
  commandMsg.r2_axis = 0;
  commandMsg.l2_axis = 0;
  commandMsg.r3_x_axis = 0;
  commandMsg.r3_y_axis = 0;
  commandMsg.l3_x_axis = 0;
  commandMsg.l3_y_axis = 0;

  responseMsg.msg_id = DRONE_TO_RADIO_MSG_ID;
  responseMsg.fw_major_v = MAJOR_VERSION;
  responseMsg.fw_minor_v = MINOR_VERSION;
  responseMsg.fw_stage_v = STAGE_VERSION;
  responseMsg.motors_armed = 0;
  responseMsg.motor1_speed = 0;
  responseMsg.motor2_speed = 0;
  responseMsg.motor3_speed = 0;
  responseMsg.motor4_speed = 0;
  responseMsg.heading = 0;
  responseMsg.pitch = 0;
  responseMsg.roll = 0;
  responseMsg.baro_altitude = 0;
}
