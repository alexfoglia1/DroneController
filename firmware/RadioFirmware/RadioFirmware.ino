#include "nRF24L01.h"
#include "RF24.h"
#include "SPI.h"
#include "proto.h"

RF24 radio(9,10);

uint64_t pipe = 0xE6E6E6E6E6E6; // Needs to be the same for communicating between 2 NRF24L01 

const byte numChars = 64;
char rxBuffer[numChars];

boolean newData = false;

void setup()
{
    for (int i = 0; i < numChars; i++)
    {
      rxBuffer[i] = 0;
    }
    Serial.begin(9600);
    Serial.println("<Arduino is ready>");
}

void loop()
{
  recvFromSerial();
  if (newData)
  {
      newData = false;
      uint32_t* msgId = (uint32_t*)(rxBuffer);
      
      if (CTRL_TO_RADIO_CFG_ID == *msgId)
      {
        txToSerial("CFG_MSG", 7);
      }
      else if (CTRL_TO_RADIO_CMD_ID == *msgId)
      {
        txToSerial("CMD_MSG", 7);
      }
      else
      {
        txToSerial("UNK_MSG", 7);
      }
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
