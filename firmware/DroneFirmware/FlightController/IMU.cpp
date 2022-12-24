#include "attitude_estimator.h"
#include "proto.h"
#include "IMU.h"

/** IMU library **/
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_LSM9DS1.h>
#include <Adafruit_Sensor.h>  // not used in this demo but required!

// i2c
Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1();

stateestimation::AttitudeEstimator attEst;

static char SENSOR_ERR_CODE;

static float _yaw;
static float _pitch;
static float _roll;
static float _temp;

static long _lastUpdate_ts;
                          //   x     y    z
float axis_sign[3]     =     {-1,   -1,  -1};
const float g = -9.81;
const float IMU_TO_FRAME_ROTATION = -M_PI/4;

void imu_to_frame(float* x, float* y, float* z)
{
  *x = g * axis_sign[0] * *x;
  *y = g * axis_sign[1] * *y;
  *z = g * axis_sign[2] * *z;
  *x = *x * cos(IMU_TO_FRAME_ROTATION) - *y * sin(IMU_TO_FRAME_ROTATION);
  *y = *x * sin(IMU_TO_FRAME_ROTATION) + *y * cos(IMU_TO_FRAME_ROTATION);
}

const float gyro_bias[3] = {0.10, -0.19, -0.06};

void initImu()
{
  SENSOR_ERR_CODE = NO_ERRORS;

  /** Forzo BARO_NO_SUCH_DEVICE per il momento : ho solo la IMU **/
  SENSOR_ERR_CODE = BARO_NO_SUCH_DEVICE;

  // Try to initialise and warn if we couldn't detect the chip
  if (!lsm.begin())
  {
     SENSOR_ERR_CODE = IMU_NO_SUCH_DEVICE;
  }
  else
  {
      // 1.) Set the accelerometer range
  lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_2G);
  
  // 2.) Set the magnetometer sensitivity
  lsm.setupMag(lsm.LSM9DS1_MAGGAIN_4GAUSS);

  // 3.) Setup the gyroscope
  lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_2000DPS);
    
    _lastUpdate_ts = -1;
    
    _yaw = 0;
    _pitch = 0;
    _roll = 0;
    _temp = 0;
  }
}

char imuErrorState()
{
  return SENSOR_ERR_CODE;
}

void updateImuFilter()
{
  if (!(SENSOR_ERR_CODE & IMU_NO_SUCH_DEVICE))
  {
    sensors_event_t a, m, g, temp;

    lsm.getEvent(&a, &m, &g, &temp); 

    float accx, accy, accz;
    float gyrox, gyroy, gyroz;
    float magnx, magny, magnz;

    accx = a.acceleration.x;
    accy = a.acceleration.y;
    accz = a.acceleration.z;

    gyrox = g.gyro.x - gyro_bias[0];
    gyroy = g.gyro.y - gyro_bias[1];
    gyroz = g.gyro.z - gyro_bias[2];

    magnx = m.magnetic.x;
    magny = m.magnetic.y;
    magnz = m.magnetic.z;
    
    imu_to_frame(&accx, &accy, &accz);
    imu_to_frame(&gyrox, &gyroy, &gyroz);
    imu_to_frame(&magnx, &magny, &magnz);
      
      if (-1 == _lastUpdate_ts)
      {
        double dt_s = 0.01;
        attEst.update(dt_s, gyrox * M_PI/180.0, gyroy * M_PI/180.0, gyroz * M_PI/180.0, accx, accy, accz, magnx, magny, magnz);
        _lastUpdate_ts = millis();
      }
      else
      {
        int t = millis();
        double dt_s = (t - _lastUpdate_ts) / 1000.0;
        attEst.update(dt_s, gyrox * M_PI/180.0, gyroy * M_PI/180.0, gyroz * M_PI/180.0, accx, accy, accz, magnx, magny, magnz);
        _lastUpdate_ts = t;
      }

      _yaw = attEst.fusedYaw();
      _pitch = attEst.fusedPitch();
      _roll = attEst.fusedRoll();

    _temp = 25;
  }
}

void getImuAttitude(float* roll, float* pitch, float* yaw)
{
  *roll = _roll;
  *pitch = _pitch;
  *yaw = 0;
}

void getImuTemperature(float* temperature)
{
  *temperature = 0;
}
