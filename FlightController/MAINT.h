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
//  float    gyro[3];
//  float    acc[3];
//  float    magn[3];
//  float    t;
//  float    dt;
  float    attitude[3];
//  uint16_t radio_channels[5];
//  uint8_t  motors_armed;
//  uint32_t motors_speed[4];
}
#ifndef WIN32
__attribute__((packed))
#endif
maint_data_t;

extern maint_data_t packet;

void MAINT_Init(uint8_t major_v, uint8_t minor_v, uint8_t stage_v);
void MAINT_UpdateIMU(float acc[3], float gyro[3], float magn[3], float t, float dt);
void MAINT_UpdateKF(float attitude[3]);
void MAINT_UpdateMOTORS(uint16_t radio_channels[5], uint8_t motors_armed, uint16_t motors_speed[4]);

maint_data_t* MAINT_Get();

#ifdef WIN32
#pragma pack(pop)
#endif

#endif //MAINT_H
