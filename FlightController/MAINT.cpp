#include "MAINT.h"

#include <string.h>

maint_data_t packet;

void MAINT_Init(uint8_t major_v, uint8_t minor_v, uint8_t stage_v)
{
  memset(&packet, 0x00, sizeof(maint_data_t));

  packet.sync = SYNC_SEQ;
  
  packet.sw_ver[0] = major_v;
  packet.sw_ver[1] = minor_v;
  packet.sw_ver[2] = stage_v;
}


void MAINT_UpdateIMU(float acc[3], float gyro[3], float magn[3], float t, float dt)
{
  //for (int i = 0; i < 3; i++)
  //{
  //  packet.acc[i]  = acc[i];
  //  packet.gyro[i] = gyro[i];
  //  packet.magn[i] = magn[i];  
  //}

  //packet.t  = t;
  //packet.dt = dt;
}


void MAINT_UpdateKF(float attitude[3])
{
  for (int i = 0; i < 3; i++)
  {
    packet.attitude[i] = attitude[i];
  }
}


void MAINT_UpdateMOTORS(uint16_t radio_channels[5], uint8_t motors_armed, uint16_t motors_speed[4])
{
  //for (int i = 0; i < 5; i++)
  //{
  //  packet.radio_channels[i] = radio_channels[i];
  //}

  //for (int i = 0; i < 4; i++)
  //{
  //  packet.motors_speed[i] = motors_speed[i];
  //}

  //packet.motors_armed = motors_armed;
}


maint_data_t* MAINT_Get()
{
  return &packet;
}
