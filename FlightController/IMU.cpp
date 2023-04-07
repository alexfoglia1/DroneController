#include <Adafruit_LSM9DS1.h>
#include <Adafruit_Sensor.h>

#include "MahonyAHRS.h"
#include "IMU.h"


Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1();
MahonyAHRS filter;
uint64_t last_update_t_micros;

uint64_t lsm9ds1_read(float acc[3], float gyro[3], float magn[3])
{
  uint64_t t = micros();
  sensors_event_t a, m, g, temp;
  lsm.getEvent(&a, &m, &g, &temp); 

  acc[X] = a.acceleration.x;
  acc[Y] = a.acceleration.y;
  acc[Z] = a.acceleration.z;

  gyro[X] = g.gyro.x;
  gyro[Y] = g.gyro.y;
  gyro[Z] = g.gyro.z;

  magn[X] = m.magnetic.x;
  magn[Y] = m.magnetic.y;
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


void IMU_Update(float acc[3], float gyro[3], float magn[3], uint64_t* dt)
{ 
  uint64_t t_micros = lsm9ds1_read(acc, gyro, magn);
  *dt = (last_update_t_micros == 0) ? 20000 : (t_micros - last_update_t_micros);
  last_update_t_micros = t_micros;
  
  filter.mahonyAHRSupdate(*dt*1e-6,
                gyro[X], gyro[Y], gyro[Z],
                acc[X],  acc[Y],  acc[Z],
                magn[X], magn[Y], magn[Z]);
}


void IMU_CurrentAttitude(float* roll, float* pitch, float* yaw)
{
  float q0 = filter.getQ1();
  float q1 = filter.getQ2();
  float q2 = filter.getQ3();
  float q3 = filter.getQ4();
  
  *roll = atan2f(q0*q1 + q2*q3, 0.5f - q1*q1 - q2*q2) * 57.295780f;
  *pitch = asinf(-2.0f * (q1*q3 - q0*q2)) * 57.295780f;
  *yaw = atan2f(q1*q2 + q0*q3, 0.5f - q2*q2 - q3*q3) * 57.295780f;
}
