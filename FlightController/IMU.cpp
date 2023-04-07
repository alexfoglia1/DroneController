#include "Adafruit_LSM9DS1.h"
#include "MadgwickAHRS.h"

#include "IMU.h"


Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1();
Madgwick filter;
uint64_t last_update_t_micros;


uint64_t lsm9ds1_read(float acc[3], float gyro[3])
{
  uint64_t t = micros();
  sensors_event_t a, g;
  lsm.getEvent(&a, &g); 

  acc[X] = -a.acceleration.x;
  acc[Y] = a.acceleration.y;
  acc[Z] = a.acceleration.z;

  gyro[X] = -g.gyro.x;
  gyro[Y] = g.gyro.y;
  gyro[Z] = g.gyro.z;

  return t;
}


bool IMU_Init()
{
  last_update_t_micros = 0;
  if (!lsm.begin())
  {
     return false;
  }
  else
  {
    lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_2G);
    lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_245DPS);
    return true;
  }
}


void IMU_UpdateKFBeta(float beta)
{
  filter.updateBeta(beta);
}


void IMU_Update(float acc[3], float gyro[3], uint64_t* dt)
{ 
  uint64_t t_micros = lsm9ds1_read(acc, gyro);
  *dt = (last_update_t_micros == 0) ? 80000.f : (t_micros - last_update_t_micros);
  last_update_t_micros = t_micros;
  
  filter.update(*dt*1e-6,
                gyro[X], gyro[Y], gyro[Z],
                acc[X],  acc[Y],  acc[Z]);
                
}


void IMU_CurrentAttitude(float* roll, float* pitch, float* yaw)
{
  *roll = (int32_t)round(filter.getRoll());
  *pitch = (int32_t)round(filter.getPitch());
  *yaw = (int32_t)round(filter.getYaw());
}
