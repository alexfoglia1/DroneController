#include <Adafruit_LSM9DS1.h>
#include <Adafruit_Sensor.h>
#include <Arduino.h>

#include "MahonyAHRS.h"
#include "IMU.h"

#define MOVING_AVG_SIZE 20


Adafruit_LSM9DS1 lsm;
MahonyAHRS ahrs;

float ax_hist[MOVING_AVG_SIZE];
float ay_hist[MOVING_AVG_SIZE];
float az_hist[MOVING_AVG_SIZE];
int ll_ax_hist;
int ll_ay_hist;
int ll_az_hist;
bool is_avg_filter_enabled;

float sampling_period_sec;


float a_filter(float f, int* ll, float* hist)
{
  if (*ll < MOVING_AVG_SIZE)
  {
    hist[*ll] = f;
    *ll += 1;
  }
  else
  {
    for (int i = 1; i < *ll; i++)
    {
      hist[i - 1] = hist[i];
    }
    hist[*ll - 1] = f;
  }

  float sum = 0.0f;
  for (int i = 0; i < *ll; i++)
  {
    sum += hist[i];
  }
  
  return sum / *ll;
}


void lsm9ds1_read(float acc[3], float gyro[3], float magn[3])
{
  sensors_event_t a, m, g, temp;
  lsm.getEvent(&a, &m, &g, &temp); 
  
  acc[X] = is_avg_filter_enabled ? a_filter(a.acceleration.x, &ll_ax_hist, &ax_hist[0]) : a.acceleration.x;
  acc[Y] = is_avg_filter_enabled ? a_filter(a.acceleration.y, &ll_ay_hist, &ay_hist[0]) : a.acceleration.y;
  acc[Z] = is_avg_filter_enabled ? a_filter(a.acceleration.z, &ll_az_hist, &az_hist[0]) : a.acceleration.z;
  
  gyro[X] = g.gyro.x;
  gyro[Y] = g.gyro.y;
  gyro[Z] = g.gyro.z;

  magn[X] = m.magnetic.x;
  magn[Y] = m.magnetic.y;
  magn[Z] = m.magnetic.z;
}


void history_reset()
{
  for (int i = 0; i < MOVING_AVG_SIZE; i++)
  {
    ax_hist[i] = 0.0f;
    ay_hist[i] = 0.0f;
    az_hist[i] = 0.0f;
    
  }
  ll_ax_hist = 0;
  ll_ay_hist = 0;
  ll_az_hist = 0;
}


bool IMU_Init(float sampling_period_s)
{
  is_avg_filter_enabled = false;
  history_reset();
  sampling_period_sec = sampling_period_s;

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


void IMU_Update(float acc[3], float gyro[3], float magn[3])
{
  lsm9ds1_read(acc, gyro, magn);
  
  ahrs.mahonyAHRSupdate(sampling_period_sec,
                gyro[X] / 57.295780f, gyro[Y] / 57.295780f, gyro[Z] / 57.295780f,
                acc[X],  acc[Y],  acc[Z],
                magn[X], magn[Y], magn[Z]);
}


void IMU_CurrentAttitude(float* roll, float* pitch, float* yaw)
{
  float q0 = ahrs.getQ1();
  float q1 = ahrs.getQ2();
  float q2 = ahrs.getQ3();
  float q3 = ahrs.getQ4();
  
  *roll = atan2f(q0*q1 + q2*q3, 0.5f - q1*q1 - q2*q2) * 57.295780f;
  *pitch = asinf(-2.0f * (q1*q3 - q0*q2)) * 57.295780f;
  *yaw = atan2f(q1*q2 + q0*q3, 0.5f - q2*q2 - q3*q3) * 57.295780f;
}


void IMU_EnableMovingAVGFilter()
{
  history_reset();
  is_avg_filter_enabled = true;
}


void IMU_DisableMovingAVGFilter()
{
  is_avg_filter_enabled = false;
}
