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
//UltraSonic ultraSonic(2,3);

CtrlToRadioCommandMessage commandMsg;
DroneToRadioResponseMessage responseMsg;
BleToUnoMessage serialMsgIn;
BleToUnoErrorMessage serialMsgInE;
char serialRxBuffer[512];
int numChars = 512;

const uint64_t rx_pipe = 0xE6E6E6E6E6E6;
const uint64_t tx_pipe = 0x6E6E6E6E6E6E;
uint8_t motorsArmed = false;
boolean armSwitched = false;

void setup(void)
{
  Serial.begin(9600);
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
  //float gndDistance = ultraSonic.distance();
  
  
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

  Serial.print("Radio available: ");
  Serial.println(radioAvailable);
  
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

    /** 
     *  TODO - Sta roba non funzionerà mai, deve essere:
     *  Sia FLIGHT_THRESHOLD v tale per cui :
     *  (Portanza = f(v)) - (peso = mg = 0.822 Kg * 9.81 ms^-2 = 8.06382 N) = 0
     *  
     *  | y
     *  |
     *  |
     *  |          ^  
     *  |          |
     *  |          | Portanza
     *  |         [X]
     *  |          |  mg
     *  |          |
     *  |          v
     *  |
     *  |
     *  |
     *  |
     *  |_____________________________ x
     *  
     *  
     *  Levetta dx in alto  -> voglio aumentare l'altitudine (tanto più in alto tanto più devo andare oltre FLIGHT_THRESHOLD)
     *  Levetta dx a zero   -> voglio restare in hovering (FLIGHT_THRESHOLD)
     *  Levetta dx in basso -> voglio diminuire l'altitudine (tanto più in basso tanto più mi avvicino a ARM_THRESHOLD)
     *  
     *  NB: NON DISARMARE MAI IN FLIGHT, FAI COME CAZZO VUOI WOW ULTRASUONI ALTIMETRO BASTA CHE PORCODDIO NON LO FAI: NON VOLARE SENZA QUESTA INFORMAZIONE (CARMELO TI ARRESTA NO CAP)
     */
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
    
    
    /** Build and send response **/
    responseMsg.motors_armed = motorsArmed ? 0x01 : 0x00;
    responseMsg.motor1_speed = DELAY_M1;
    responseMsg.motor2_speed = DELAY_M2;
    responseMsg.motor3_speed = DELAY_M3;
    responseMsg.motor4_speed = DELAY_M4;

    //boolean dataFromSerial = recvFromSerial();
    if (false)
    {
      uint8_t* msgId = (uint8_t*)(serialRxBuffer);
      
      if (BLE_TO_UNO_MSG_ID == *msgId)
      {
        BleToUnoMessage* msgIn = (BleToUnoMessage*)(serialRxBuffer);
        serialMsgIn.msg_id = msgIn->msg_id;
        serialMsgIn.pitch = msgIn->pitch;
        serialMsgIn.roll = msgIn->roll;
        serialMsgIn.yaw = msgIn->yaw;
        serialMsgIn.baro_altitude = msgIn->baro_altitude;
      }
      else
      {
        /** Ignore error handling for the moment **/
      }
    }
        
    responseMsg.heading = floatRadiansToUint16Degrees(serialMsgIn.yaw);
    responseMsg.roll = floatRadiansToUint16Degrees(serialMsgIn.roll);
    responseMsg.pitch = floatRadiansToUint16Degrees(serialMsgIn.pitch);
    responseMsg.baro_altitude = serialMsgIn.baro_altitude;
    //responseMsg.gnd_distance = gndDistance;
    
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

  serialMsgIn.msg_id = 0;
  serialMsgIn.pitch = 0.f;
  serialMsgIn.roll = 0.f;
  serialMsgIn.yaw = 0.f;
  serialMsgIn.baro_altitude = 0;

  serialMsgInE.msg_id = 0;
  serialMsgInE.error_code = 0;
}

boolean recvFromSerial()
{
    static boolean recvInProgress = false;
    static byte ndx = 0;
    boolean dataFromSerial = false;
    char rc;
 
    while (Serial.available() > 0)
    {
        rc = Serial.read();

        if (recvInProgress == true)
        {
            if (rc != endMarker)
            {
                serialRxBuffer[ndx] = rc;
                ndx++;
                if (ndx >= numChars)
                {
                    ndx = numChars - 1;
                }
            }
            else
            {
                recvInProgress = false;
                dataFromSerial = true;
                ndx = 0;
            }
        }

        else if (rc == startMarker)
        {
            recvInProgress = true;
        }
    }

    return dataFromSerial;
}

void txToSerial(char* data, int len)
{
  char txBuffer[len + 2];
  txBuffer[0] = startMarker;
  for (int i = 0; i < len; i++)
  {
    txBuffer[i + 1] = data[i];
  }
  txBuffer[len + 1] = endMarker;

  Serial.write(txBuffer, len + 2);
  delay(10);
}

uint16_t floatRadiansToUint16Degrees(float angleRad)
{
  float angleDeg = angleRad * 180.0 / M_PI;
  float bitResolution = 0xFFFF / 360.f;

  uint16_t converted = angleDeg * bitResolution;
  return converted;
}
