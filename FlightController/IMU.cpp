#include <Adafruit_LSM9DS1.h>
#include <Adafruit_Sensor.h>
#include <MadgwickAHRS.h>

#include "IMU.h"


Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1();
Madgwick filter;
uint64_t last_update_t_micros;

//IMU to BODY x,y axis swapped
uint64_t lsm9ds1_read(float acc[3], float gyro[3], float magn[3])
{
  uint64_t t = micros();
  sensors_event_t a, m, g, temp;
  lsm.getEvent(&a, &m, &g, &temp); 

  acc[X] = a.acceleration.y;
  acc[Y] = a.acceleration.x;
  acc[Z] = a.acceleration.z;

  gyro[X] = g.gyro.y;
  gyro[Y] = g.gyro.x;
  gyro[Z] = g.gyro.z;

  magn[X] = m.magnetic.y;
  magn[Y] = m.magnetic.x;
  magn[Z] = m.magnetic.z;

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
    lsm.setupMag(lsm.LSM9DS1_MAGGAIN_4GAUSS);
    lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_245DPS);

    return true;
  }
}


void IMU_UpdateKFBeta(float beta)
{
  filter.updateBeta(beta);
}


void IMU_Update(float acc[3], float gyro[3], float magn[3], uint64_t* dt)
{ 
  uint64_t t_micros = lsm9ds1_read(acc, gyro, magn);
  *dt = (last_update_t_micros == 0) ? 1000 : (t_micros - last_update_t_micros);
  last_update_t_micros = t_micros;
  
  filter.update(*dt*1e-6,
                gyro[X], gyro[Y], gyro[Z],
                acc[X],  acc[Y],  acc[Z],
                magn[X], magn[Y], magn[Z]);
}


void IMU_CurrentAttitude(float* roll, float* pitch, float* yaw)
{
  *roll = (filter.getRoll());
  *pitch = (filter.getPitch());
  *yaw = (filter.getYaw());
}


void IMU_CurrentVariance(float* roll, float* pitch, float* yaw)
{
  *roll = (filter.getRollVariance());
  *pitch = (filter.getPitchVariance());
  *yaw = (filter.getYawVariance());
}
