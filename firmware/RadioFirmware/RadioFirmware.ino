#include "nRF24L01.h"
#include "RF24.h"
#include "SPI.h"
#include "proto.h"

/**
 * Lo sketch riceve in maniera asincrona 2 tipi di messaggi:
 * msg_id(CTRL_TO_RADIO_CFG_ID): Cambia configurazione
 * msg_id(CTRL_TO_RADIO_CMD_ID): Cambia messaggio tx radio
 * A questi messaggi rispondo sempre con un msg_id(RADIO_TO_CTRL_ACK_ID)
 * 
 * Lo sketch invia con frequenza:
 * TX_CONFIG_FREQUENCY_HZ msg_id(RADIO_TO_CTRL_CFG_ID): configurazione attuale, su seriale
 * TX_ECHO_FREQUENCY_HZ msg_id(RADIO_TO_CTRL_CMD_ID): messaggio tx radio attuale, su seriale
 * TX_RADIO_FREQUENCY_HZ msg_id(RADIO_TO_DRONE_CMD_ID): messaggio tx radio, su radio
 * 
 */

#define MAJOR_VERSION '0'
#define MINOR_VERSION '1'
#define STAGE_VERSION 'b'
#define TX_CONFIG_FREQUENCY_HZ 1
#define TX_ECHO_FREQUENCY_HZ   10
#define TX_RADIO_FREQUENCY_HZ  100


RF24 radio(9,10);

uint64_t tx_pipe = 0x00; // Needs to be the same for communicating between 2 NRF24L01 
uint64_t rx_pipe = 0x00; // Needs to be the same for communicating between 2 NRF24L01 

const byte numChars = 128;
char serialRxBuffer[numChars];

CtrlToRadioCommandMessage lastCmdMessage;

boolean newData = false;
boolean sendAck = false;

long count_to_cfg = 0;
long count_to_echo = 0;
long count_to_radio = 0;

const long cfgPeriod_us = 1e6 / TX_CONFIG_FREQUENCY_HZ;
const long echoPeriod_us = 1e6 / TX_ECHO_FREQUENCY_HZ;
const long radioPeriod_us = 1e6 / TX_RADIO_FREQUENCY_HZ;

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
    
    Serial.begin(9600);
    radio.begin();
    
    RadioToCtrlAliveMessage alive;
    alive.msg_id  = RADIO_TO_CTRL_ALIVE_ID;
    alive.major_v = MAJOR_VERSION;
    alive.minor_v = MINOR_VERSION;
    alive.stage_v = STAGE_VERSION;
    
    txToSerial((char*)&alive, sizeof(RadioToCtrlAliveMessage));
}

void loop()
{
  recvFromSerial();
  RadioToCtrlAckMessage ack;
  ack.msg_id = RADIO_TO_CTRL_ACK_ID;
  RadioToCtrlConfigMessage configMsg;
  configMsg.msg_id = RADIO_TO_CTRL_CFG_ID;
  
  if (newData)
  {
      newData = false;
      uint32_t* msgId = (uint32_t*)(serialRxBuffer);
      ack.msg_acked = *msgId;
      
      if (CTRL_TO_RADIO_CFG_ID == *msgId)
      {
        CtrlToRadioConfigMessage* msgIn = (CtrlToRadioConfigMessage*)(serialRxBuffer);
        tx_pipe = msgIn->tx_pipe;
        rx_pipe = msgIn->rx_pipe;
        ack.ack_status = 1;
        radio.openWritingPipe(tx_pipe); // Get NRF24L01 ready to transmit
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
        
        ack.ack_status = 1;
      }
      else
      {
        ack.ack_status = 0;
      }

      /** Mando un ack **/
      sendAck = true;
  }
  
  /** Mando configurazione **/
  configMsg.tx_pipe = tx_pipe;
  configMsg.rx_pipe = rx_pipe;
  count_to_cfg += 1;
  if (cfgPeriod_us == count_to_cfg)
  {
    txToSerial((char*)&configMsg, sizeof(RadioToCtrlConfigMessage));
    count_to_cfg = 0;
  }

  /** Inoltro su radio il comando attuale **/
  count_to_radio += 1;
  if (radioPeriod_us == count_to_radio)
  {
    lastCmdMessage.msg_id = RADIO_TO_DRONE_CMD_ID;
    radio.write((char*)&lastCmdMessage, sizeof(CtrlToRadioCommandMessage));
    count_to_radio = 0;
  }
  
  /** Mando echo del comando attuale **/
  count_to_echo += 1;
  if (echoPeriod_us == count_to_echo)
  {
    lastCmdMessage.msg_id = RADIO_TO_CTRL_CMD_ID;
    txToSerial((char*)&lastCmdMessage, sizeof(CtrlToRadioCommandMessage));
    count_to_echo = 0;
  }

  if (sendAck)
  {
    txToSerial((char*)&ack, sizeof(RadioToCtrlAckMessage));
    sendAck = false;
  }
  
  delayMicroseconds(1);
}

void recvFromSerial()
{
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char rc;
 
    while (Serial.available() > 0 && newData == false)
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
                ndx = 0;
                newData = true;
            }
        }

        else if (rc == startMarker)
        {
            recvInProgress = true;
        }
    }
}

void txToSerial(char* data, int len)
{
  Serial.write(startMarker);
  for (int i = 0; i < len; i++)
  {
    Serial.write(data[i]);
  }
  Serial.write(endMarker);
}

void handleCfgMsg()
{

}

void handleCmdMsg()
{

}

void handleUnknownMsgId(uint32_t msgId)
{

}
