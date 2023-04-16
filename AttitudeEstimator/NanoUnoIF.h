#ifndef SERIALPROTO_H
#define SERIALPROTO_H

typedef struct
{
    uint8_t  sync;
    uint8_t  lsm9ds1_found;
    float    roll;
    float    pitch;
    float    yaw;
    uint32_t loop_time;
    uint8_t  avg_filter_enabled;
    uint8_t  checksum;
}__attribute__((packed)) NANO2UNO_Message;


inline uint8_t NANO2UNO_Cks(uint8_t* buf)
{
  uint32_t n_bytes = sizeof(NANO2UNO_Message) - 1;
  uint8_t cks = 0;
  for (uint32_t i = 0; i < n_bytes; i++)
  {
    cks ^= buf[i];
  }

  return cks;
}

#endif
