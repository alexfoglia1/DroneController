#ifndef MAINT_H
#define MAINT_H

#include <stdint.h>

#define SYNC_SEQ 0xB0BAFE77

#ifdef WIN32
#pragma pack(push, 1)
#endif

typedef struct
{
  uint32_t sync;
  uint8_t  sw_ver[3];
  uint8_t  avg_filter; 
  float    attitude[3];
  float    throttle;
  float    command[3];
  float    PID[3];
  uint8_t  motors_armed;
  uint16_t motors_speed[4];
  uint64_t loop_time;
}
#ifndef WIN32
__attribute__((packed))
#endif
maint_data_t;

void MAINT_Init(uint8_t major_v, uint8_t minor_v, uint8_t stage_v);
void MAINT_UpdateMovingAVGFilterState(uint8_t is_avg_filter_enabled);
void MAINT_UpdateAHRS(float attitude[3]);
void MAINT_UpdateCMD(float throttle, float cmd[3]);
void MAINT_UpdatePID(float pid[3]);
void MAINT_UpdateMOTORS(uint8_t motors_armed, uint16_t motors_speed[4]);
void MAINT_UpdateLoopTime(uint64_t loop_time_us);

maint_data_t* MAINT_Get();

#ifdef WIN32
#pragma pack(pop)
#endif

#endif //MAINT_H
