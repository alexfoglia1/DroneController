#include "nRF24L01.h"
#include "RF24.h"
#include "SPI.h"
#include "proto.h"

#include <Servo.h>

#define MAX_SIGNAL 2000
#define MIN_SIGNAL 1000
#define ARM_THRESHOLD 100
#define MOTOR_PIN1 8
#define MOTOR_PIN2 7
#define MOTOR_PIN3 6
#define MOTOR_PIN4 5

#define RADIO_RX_FREQ 100
#define MAX_PKT_LOSS  10

#define APP_NAME "Arduino Flight Controller"
#define MAJOR_VERSION '0'
#define MINOR_VERSION '1'
#define STAGE_VERSION 'b'

Servo motor1;
Servo motor2;
Servo motor3;
Servo motor4;

CtrlToRadioCommandMessage commandMsg;
DroneToRadioResponseMessage responseMsg;

RF24 radio(9,10);

const uint64_t rx_pipe = 0xE6E6E6E6E6E6;
const uint64_t tx_pipe = 0x6E6E6E6E6E6E;
const uint64_t radioRxPeriod_micros = 1e6 / RADIO_RX_FREQ;
uint64_t countToRxTimeout = 0;

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
  radio.openWritingPipe(tx_pipe);
  radio.openReadingPipe(1, rx_pipe);
}

void loop(void)
{
  /** Read from radio and apply to motors **/
  boolean radioRx = false;
  while (radio.available())
  {
    radio.read((char*)&commandMsg, sizeof(CtrlToRadioCommandMessage));
    radioRx = true;
  }

  int deltaTimeout;
  if (!radioRx)
  {
    deltaTimeout = 1;
  }
  else
  {
    deltaTimeout = 0;
  }

  if (countToRxTimeout > MAX_PKT_LOSS * radioRxPeriod_micros)
  {
    countToRxTimeout = 0;
    //clearMessages();
  }
  
  int DELAY = 0;
  if (RADIO_TO_DRONE_MSG_ID == commandMsg.msg_id)
  {
    if (radioRx)
    {
      countToRxTimeout = 0;
    }
    DELAY = jsAxisToSpeed(commandMsg.r2_axis);
  }
  else
  {
   DELAY = MIN_SIGNAL;
  }
   
  motor1.writeMicroseconds(DELAY);
  motor2.writeMicroseconds(DELAY);
  motor3.writeMicroseconds(DELAY);
  motor4.writeMicroseconds(DELAY);
   
  float SPEED = (DELAY - 1000) / 10;
  Serial.print("\n");
  Serial.println("Motor speed:"); Serial.print("  "); Serial.print(SPEED); Serial.print("%");
  countToRxTimeout += (4 * DELAY) * deltaTimeout;

  /** Todo: read from BLE Sense sensors state **/

  /** Build and send response **/

  responseMsg.echoed = commandMsg;
  responseMsg.motor1_speed = DELAY;
  responseMsg.motor2_speed = DELAY;
  responseMsg.motor3_speed = DELAY;
  responseMsg.motor4_speed = DELAY;
  responseMsg.heading = 0;
  responseMsg.pitch = 0;
  responseMsg.roll = 0;
  responseMsg.baro_altitude = 0;
  
  radio.stopListening();
  radio.write((char*)&responseMsg, sizeof(DroneToRadioResponseMessage));
  radio.startListening();
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

int jsAxisToSpeed(uint8_t jsAxis)
{
  float jsAxisPercentage = (float)jsAxis / 255.f;
  int signalSpan = MAX_SIGNAL - MIN_SIGNAL;
  return (MIN_SIGNAL + ARM_THRESHOLD) + ( (signalSpan - ARM_THRESHOLD) * jsAxisPercentage);
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
  responseMsg.echoed = commandMsg;
  responseMsg.motor1_speed = 0;
  responseMsg.motor2_speed = 0;
  responseMsg.motor3_speed = 0;
  responseMsg.motor4_speed = 0;
  responseMsg.heading = 0;
  responseMsg.pitch = 0;
  responseMsg.roll = 0;
  responseMsg.baro_altitude = 0;
}
