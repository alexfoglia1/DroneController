
#include <Wire.h>
#include <NeoSWSerial.h>

#include "IMU.h"
#include "NanoUnoIF.h"


#define MAJOR_VERSION '1'
#define MINOR_VERSION '0'
#define STAGE_VERSION 'B'

#define MOTORS_ARMED_PIN 10
#define SW_UART_RX 11
#define SW_UART_TX 12

#define SAMPLING_PERIOD_S 0.01f
#define SAMPLING_PERIOD_US 10000UL
#define SECONDS_TO_MICROSECONDS 1000000UL



NeoSWSerial uart_uno(SW_UART_RX, SW_UART_TX);
NANO2UNO_Message message_out;

int next_byte_out;

uint64_t last_exec_micros;
uint64_t loop_time;
bool lsm9ds1_found;
float attitude[3];



void setup(void)
{
  pinMode(MOTORS_ARMED_PIN, INPUT);
  
  Serial.begin(115200);
  
  uart_uno.begin(38400);
  
  lsm9ds1_found = IMU_Init(SAMPLING_PERIOD_S);
  IMU_EstimateBias(100);

  last_exec_micros = 0;
  loop_time = 0;

  attitude[0] = 0.0f;
  attitude[1] = 0.0f;
  attitude[2] = 0.0f;

  message_out.sync = 0xFF;
  message_out.lsm9ds1_found = lsm9ds1_found;
  message_out.roll = 0.0f;
  message_out.pitch = 0.0f;
  message_out.yaw = 0.0f;
  message_out.loop_time = 0;
  message_out.avg_filter_enabled = 0;
  message_out.checksum = NANO2UNO_Cks((uint8_t*)&message_out);

  next_byte_out = 0;
}

void loop(void)
{ 
// ----------------------------------------------------------------------------------------------------------------- 
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
      delayMicroseconds(remaining_micros);
    }
  }
  uint64_t cur_micros = micros();
  loop_time = cur_micros - last_exec_micros;
  last_exec_micros = cur_micros;
// ----------------------------------------------------------------------------------------------------------------- 
// --------------------------------------------- READ CURRENT ATTITUDE --------------------------------------------- 
  bool motors_armed = (digitalRead(MOTORS_ARMED_PIN) == HIGH);
  if (lsm9ds1_found)
  {

    float acc[3]  = {0.f, 0.f, 0.f};
    float gyro[3] = {0.f, 0.f, 0.f};
    float magn[3] = {0.f, 0.f, 0.f};

    // Check if avg filter shall be used
    
    if (motors_armed)
    {
      IMU_EnableMovingAVGFilter();
    }
    else
    {
      IMU_DisableMovingAVGFilter();
    }
    
    // Read IMU and update AHRS
    IMU_Update(acc, gyro, magn);
    // ----------------------------------
    // Get current attitude
    IMU_CurrentAttitude(&attitude[0], &attitude[1], &attitude[2]);
  }
// ---------------------------------------------- UPDATE MESSAGE OUT -----------------------------------------------
  if (next_byte_out == 0)
  {
    message_out.loop_time = loop_time;
    message_out.roll = attitude[0];
    message_out.pitch = attitude[1];
    message_out.yaw = attitude[2];
    message_out.avg_filter_enabled = motors_armed ? 1 : 0;
  }
// -----------------------------------------------------------------------------------------------------------------
// ---------------------------------------------- WRITE MESSAGE OUT ------------------------------------------------
  if (next_byte_out == sizeof(NANO2UNO_Message) - 1)
  {
    // Calc checksum
    message_out.checksum = NANO2UNO_Cks((uint8_t*)&message_out);
    // ----------------------------------
  }
  uint8_t* p_message_out = (uint8_t*)(&message_out) + next_byte_out;
  uart_uno.write(*p_message_out);
  next_byte_out += 1;

  if (next_byte_out == sizeof(NANO2UNO_Message))
  {
    next_byte_out = 0;
  }
// -----------------------------------------------------------------------------------------------------------------
}
