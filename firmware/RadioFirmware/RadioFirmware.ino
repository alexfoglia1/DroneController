#include "nRF24L01.h"
#include "RF24.h"
#include "SPI.h"
#include "proto.h"

#define MAJOR_VERSION '0'
#define MINOR_VERSION '1'
#define STAGE_VERSION 'b'

RF24 radio(9,10);

uint64_t tx_pipe = 0x00; // Needs to be the same for communicating between 2 NRF24L01 
uint64_t rx_pipe = 0x00; // Needs to be the same for communicating between 2 NRF24L01 

const byte numChars = 128;

char rxBuffer[numChars];

boolean newData = false;

void setup()
{
    for (int i = 0; i < numChars; i++)
    {
      rxBuffer[i] = 0;
    }
    
    Serial.begin(9600);
    
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
  if (newData)
  {
      newData = false;
      uint32_t* msgId = (uint32_t*)(rxBuffer);
      ack.msg_acked = *msgId;
      
      if (CTRL_TO_RADIO_CFG_ID == *msgId)
      {
        CtrlToRadioConfig* msgIn = (CtrlToRadioConfig*)(rxBuffer);
        tx_pipe = msgIn->tx_pipe;
        rx_pipe = msgIn->rx_pipe;
        ack.ack_status = 1;
      }
      else if (CTRL_TO_RADIO_CMD_ID == *msgId)
      {
        ack.ack_status = 1;
      }
      else
      {
        ack.ack_status = 0;
      }

      /** Per il momento mando solo un ack **/
      txToSerial((char*)&ack, sizeof(RadioToCtrlAckMessage));
  }
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
                rxBuffer[ndx] = rc;
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
