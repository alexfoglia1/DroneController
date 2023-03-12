#include <Adafruit_LSM9DS1.h>
#include <Adafruit_Sensor.h>
#include <MadgwickAHRS.h>
#include <math.h>

#include "IMU.h"

Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1();
Madgwick filter;
long last_update_t;


void imu_to_frame(vector3d_t* v)
{
  v->x = v->x * cos(IMU_TO_FRAME_ROTATION) - v->y * sin(IMU_TO_FRAME_ROTATION);
  v->y = v->x * sin(IMU_TO_FRAME_ROTATION) + v->y * cos(IMU_TO_FRAME_ROTATION);
}


float lsm9ds1_read(vector3d_t* acc, vector3d_t* gyro, vector3d_t* magn)
{
  float t = micros();
  sensors_event_t a, m, g, temp;
  lsm.getEvent(&a, &m, &g, &temp); 

  acc->x = a.acceleration.x;
  acc->y = a.acceleration.y;
  acc->z = a.acceleration.z;

  gyro->x = g.gyro.x;
  gyro->y = g.gyro.y;
  gyro->z = g.gyro.z;

  magn->x = m.magnetic.x;
  magn->y = m.magnetic.y;
  magn->z = m.magnetic.z;

  imu_to_frame(acc);
  imu_to_frame(gyro);
  imu_to_frame(magn);

  return t;
}


bool IMU_Init()
{ 
  last_update_t = -1;
  if (!lsm.begin())
  {
     return false;
  }
  else
  {
    lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_2G);
    lsm.setupMag(lsm.LSM9DS1_MAGGAIN_4GAUSS);
    lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_2000DPS);

    return true;
  }
}


void IMU_Update()
{
  vector3d_t accelerometer = {0.0f, 0.0f, 0.0f};
  vector3d_t gyroscope     = {0.0f, 0.0f, 0.0f};
  vector3d_t magnetometer  = {0.0f, 0.0f, 0.0f};
  
  float t = lsm9ds1_read(&accelerometer, &gyroscope, &magnetometer);
  float dt = (last_update_t < 0) ? 0.1f : (t - last_update_t);
  last_update_t = t;
  
  filter.update(dt / 1000000.0f,
                gyroscope.x,     gyroscope.y,     gyroscope.z,
                accelerometer.x, accelerometer.y, accelerometer.z,
                magnetometer.x,  magnetometer.y,  magnetometer.z);
}


void IMU_CurrentAttitude(float* roll, float* pitch, float* yaw)
{
  *roll = filter.getRoll();
  *pitch = filter.getPitch();
  *yaw = filter.getYaw();
}
