#include "MAINT.h"

#include <string.h>

maint_data_t MAINT_packet;

void MAINT_Init(uint8_t major_v, uint8_t minor_v, uint8_t stage_v)
{
  memset(&MAINT_packet, 0x00, sizeof(maint_data_t));

  MAINT_packet.sync = SYNC_SEQ;
  
  MAINT_packet.sw_ver[0] = major_v;
  MAINT_packet.sw_ver[1] = minor_v;
  MAINT_packet.sw_ver[2] = stage_v;

  MAINT_packet.bw_filter_state = 0;
}


void MAINT_UpdateIMU(float acc[3], float gyro[3], float magn[3])
{
  for (int i = 0; i < 3; i++)
  {
    MAINT_packet.acc[i]  = acc[i];
    MAINT_packet.gyro[i] = gyro[i];
    MAINT_packet.magn[i] = magn[i];  
  }
}


void MAINT_UpdateButterworthFilterState(uint8_t bw_filter_state)
{
  MAINT_packet.bw_filter_state = bw_filter_state;
}


void MAINT_UpdateKF(uint64_t dt, float attitude[3])
{
  for (int i = 0; i < 3; i++)
  {
    MAINT_packet.attitude[i] = attitude[i];
  }

  MAINT_packet.kf_dt = dt;
}


void MAINT_UpdateCMD(float throttle, float cmd[3])
{
  for (int i = 0; i < 3; i++)
  {
    MAINT_packet.command[i] = cmd[i];
  }
  MAINT_packet.throttle = throttle;
}


void MAINT_UpdatePID(float pid[3])
{
  for (int i = 0; i < 3; i++)
  {
    MAINT_packet.PID[i] = pid[i];
  }
}


void MAINT_UpdateMOTORS(uint8_t motors_armed, uint16_t motors_speed[4])
{
  for (int i = 0; i < 4; i++)
  {
    MAINT_packet.motors_speed[i] = motors_speed[i];
  }

  MAINT_packet.motors_armed = motors_armed;
}


maint_data_t* MAINT_Get()
{
  return &MAINT_packet;
}
