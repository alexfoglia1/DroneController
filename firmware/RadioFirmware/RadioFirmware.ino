#include "nRF24L01.h"
#include "RF24.h"
#include "SPI.h"
#include "proto.h"

/**
 * Lo sketch riceve in maniera asincrona 2 tipi di messaggi:
 * msg_id(CTRL_TO_RADIO_CFG_ID): Cambia configurazione
 * msg_id(CTRL_TO_RADIO_CMD_ID): Cambia messaggio tx radio
 * 
 * Lo sketch invia con frequenza 33Hz (3 sleep totali di 10 milli)
 * msg_id(RADIO_TO_CTRL_ALIVE_ID): keep alive
 * msg_id(RADIO_TO_CTRL_CFG_ID): configurazione attuale, su seriale
 * msg_id(RADIO_TO_CTRL_ECHO_ID): messaggio tx radio attuale, su seriale
 * msg_id(RADIO_TO_DRONE_CMD_ID): messaggio tx radio, su radio
 * 
 */

#define MAJOR_VERSION '0'
#define MINOR_VERSION '1'
#define STAGE_VERSION 'b'


RF24 radio(9,10);

uint64_t rx_pipe = 0; // Needs to be the same for communicating between 2 NRF24L01 
uint64_t tx_pipe = 0; // Needs to be the same for communicating between 2 NRF24L01 
bool configured = false;

const byte numChars = 128;
char serialRxBuffer[numChars];

CtrlToRadioCommandMessage   lastCmdMessage;
DroneToRadioResponseMessage lastDroneResponse;
RadioToCtrlAliveMessage     alive;


void setup()
{
    for (int i = 0; i < numChars; i++)
    {
      serialRxBuffer[i] = 0;
    }

    lastCmdMessage.l2_axis = 0;
    lastCmdMessage.r2_axis = 0;
    lastCmdMessage.l3_x_axis = 0;
    lastCmdMessage.r3_y_axis = 0;
    lastCmdMessage.r3_x_axis = 0;
    lastCmdMessage.r3_y_axis = 0;

    lastDroneResponse.echoed = lastCmdMessage;
    lastDroneResponse.motor1_speed = 0;
    lastDroneResponse.motor2_speed = 0;
    lastDroneResponse.motor3_speed = 0;
    lastDroneResponse.motor4_speed = 0;
    lastDroneResponse.heading = 0;
    lastDroneResponse.pitch = 0;
    lastDroneResponse.roll = 0;
    lastDroneResponse.baro_altitude = 0;
    
    radio.begin();
    
    Serial.begin(115200); 
     
    alive.msg_id  = RADIO_TO_CTRL_ALIVE_ID;
    alive.major_v = MAJOR_VERSION;
    alive.minor_v = MINOR_VERSION;
    alive.stage_v = STAGE_VERSION;
}
  
void loop()
{
  txToSerial((char*)&alive, sizeof(RadioToCtrlAliveMessage));
      
  boolean dataFromSerial = recvFromSerial();
  
  if (dataFromSerial)
  {
      dataFromSerial = false;
      uint32_t* msgId = (uint32_t*)(serialRxBuffer);
      
      if (CTRL_TO_RADIO_CFG_ID == *msgId)
      {
        CtrlToRadioConfigMessage* msgIn = (CtrlToRadioConfigMessage*)(serialRxBuffer);
        tx_pipe = msgIn->tx_pipe;
        rx_pipe = msgIn->rx_pipe;
        if (0 == msgIn->config_ok)
        {
          configured = false;
        }
        else
        {
          configured = true;
          radio.openWritingPipe(tx_pipe);
          //radio.openReadingPipe(1, rx_pipe);

        }
      }
      else if (CTRL_TO_RADIO_CMD_ID == *msgId)
      {
        CtrlToRadioCommandMessage* msgIn = (CtrlToRadioCommandMessage*)(serialRxBuffer);

        lastCmdMessage.l2_axis = msgIn->l2_axis;
        lastCmdMessage.r2_axis = msgIn->r2_axis;
        lastCmdMessage.l3_x_axis = msgIn->l3_x_axis;
        lastCmdMessage.l3_y_axis = msgIn->l3_y_axis;
        lastCmdMessage.r3_x_axis = msgIn->r3_x_axis;
        lastCmdMessage.r3_y_axis = msgIn->r3_y_axis;
      }
  }

  /** Mando configurazione su seriale **/
  RadioToCtrlConfigMessage configMsg;
  configMsg.msg_id  = RADIO_TO_CTRL_CFG_ID;
  configMsg.tx_pipe = tx_pipe;
  configMsg.rx_pipe = rx_pipe;
  txToSerial((char*)&configMsg, sizeof(RadioToCtrlConfigMessage));

  /** Inoltro su radio il comando attuale **/
  lastCmdMessage.msg_id = RADIO_TO_DRONE_MSG_ID;
  if (configured == true)
    radio.write((char*)&lastCmdMessage, sizeof(CtrlToRadioCommandMessage));

  
  /** Aspetto la risposta del drone **/
  //if (configured)
  //{
   // if (radio.isAckPayloadAvailable())
   // {
   //   radio.read((char*)&lastDroneResponse, sizeof(DroneToRadioResponseMessage));
    //}
  //}
  
  /** Mando risposta del drone a controller **/
  lastDroneResponse.msg_id = RADIO_TO_CTRL_ECHO_ID;
  txToSerial((char*)&lastDroneResponse, sizeof(DroneToRadioResponseMessage));
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
