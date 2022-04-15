#include "nRF24L01.h"
#include "RF24.h"
#include "SPI.h"
#include "proto.h"

RF24 radio(9,10);

const uint64_t pipe = 0xE6E6E6E6E6E6; // Needs to be the same for communicating between 2 NRF24L01 

const byte numChars = 512;
char receivedChars[numChars];

boolean newData = false;

void setup()
{
    Serial.begin(9600);
    Serial.println("<Arduino is ready>");
}

void loop()
{
    recvFromSerial();
    
    if (newData == true)
    {
      uint32_t* msgId = (uint32_t*)(receivedChars);
      if (CTRL_TO_RADIO_CFG_ID == *msgId)
      {
        handleCfgMsg();
      }
      else if (CTRL_TO_RADIO_CMD_ID == *msgId)
      {
        handleCmdMsg();
      }
      else
      {
        handleUnknownMsgId(*msgId);
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
                receivedChars[ndx] = rc;
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

void handleCfgMsg()
{
  Serial.println("Received config msg!");
}

void handleCmdMsg()
{
  Serial.println("Received command msg!");
}

void handleUnknownMsgId(uint32_t msgId)
{
  Serial.print("Received unknown msg id: ");
  Serial.println(msgId);
}
