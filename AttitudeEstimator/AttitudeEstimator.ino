
#include <Wire.h>
#include <NeoSWSerial.h>

#include "IMU.h"
#include "NanoUnoIF.h"


#define MAJOR_VERSION '1'
#define MINOR_VERSION '0'
#define STAGE_VERSION 'B'

#define SW_UART_RX 11
#define SW_UART_TX 12

#define SAMPLING_PERIOD_S 0.01f
#define SAMPLING_PERIOD_US 10000UL
#define SECONDS_TO_MICROSECONDS 1000000UL


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

typedef enum
{
  WAIT_SYNC,
  WAIT_DATA  
} uno_proto_state_t;

uint64_t last_exec_micros;
uint64_t loop_time;
drone_attitude_t attitude;
bool lsm9ds1_found;
NANO2UNO_Message message_out;
UNO2NANO_Message message_in;
uno_proto_state_t rx_status;
NeoSWSerial uart_uno(SW_UART_RX, SW_UART_TX);
int next_byte_out;

void update_fsm(uint8_t byte_in)
{
  switch (rx_status)
  {
    case WAIT_SYNC:
    {
      if (0xFF == byte_in)
      {
        message_in.sync = byte_in;
        rx_status = WAIT_DATA;
      }
      break;
    }
    case WAIT_DATA:
    {
      message_in.motors_armed = byte_in;
      rx_status = WAIT_SYNC;
      break;
    }
  }
}


void setup(void)
{
  Serial.begin(115200);
  
  uart_uno.begin(38400);
  
  lsm9ds1_found = IMU_Init(SAMPLING_PERIOD_S);
  IMU_EstimateBias(100);
  
  attitude.data.pitch = 0;
  attitude.data.roll  = 0;
  attitude.data.yaw   = 0;

  last_exec_micros = 0;
  loop_time = 0;

  message_out.sync = 0xFF;
  message_out.lsm9ds1_found = lsm9ds1_found;
  message_out.roll = 0.0f;
  message_out.pitch = 0.0f;
  message_out.yaw = 0.0f;
  message_out.loop_time = 0;

  message_in.sync = 0x00;
  message_in.motors_armed = 0x00;

  rx_status = WAIT_SYNC;
  next_byte_out = 0;
}

void loop(void)
{ 
// ---------------------------------------------- UPDATE MESSAGE IN ------------------------------------------------
  int byte_in = uart_uno.read();
  if (byte_in > 0)
  {
    update_fsm((uint8_t)(byte_in & 0xFF));
  }
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
  if (lsm9ds1_found)
  {
// --------------------------------------------- READ CURRENT ATTITUDE --------------------------------------------- 
    float acc[3]  = {0.f, 0.f, 0.f};
    float gyro[3] = {0.f, 0.f, 0.f};
    float magn[3] = {0.f, 0.f, 0.f};

    // Check if avg filter shall be used
    if (message_in.motors_armed)
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
    IMU_CurrentAttitude(&attitude.data.roll, &attitude.data.pitch, &attitude.data.yaw);
    // ----------------------------------
    // Update message out
    if (next_byte_out == 0)
    {
      message_out.loop_time = loop_time;
      message_out.roll = attitude.data.roll;
      message_out.pitch = attitude.data.pitch;
      message_out.yaw = attitude.data.yaw;
    }
    // ----------------------------------
  }
// -----------------------------------------------------------------------------------------------------------------
// ---------------------------------------------- WRITE MESSAGE OUT ------------------------------------------------
  if (next_byte_out == sizeof(NANO2UNO_Message) - 1)
  {
    // Calc checksum
    message_out.checksum = NANO2UNO_Cks((uint8_t*)&message_out);
    Serial.print("TX checksum: ");
    Serial.println(message_out.checksum);
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
