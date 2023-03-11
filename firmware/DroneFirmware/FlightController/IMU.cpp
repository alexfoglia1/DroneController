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

static float _gyro_x;
static float _gyro_y;
static float _gyro_z;

static float _yaw;
static float _pitch;
static float _roll;

static long _lastUpdate_ts;


const float gyro_bias[3] = {-0.26f, -0.64f, 0.0f};
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

void read_imu_raw(float* acc_x,  float* acc_y,  float* acc_z,
                  float* gyro_x, float* gyro_y, float* gyro_z,
                  float* magn_x, float* magn_y, float* magn_z)
{
    sensors_event_t a, m, g, temp;

    lsm.getEvent(&a, &m, &g, &temp); 

    if (acc_x) *acc_x = a.acceleration.x;
    if (acc_y) *acc_y = a.acceleration.y;
    if (acc_z) *acc_z = a.acceleration.z;

    if (gyro_x) *gyro_x = g.gyro.x;
    if (gyro_y) *gyro_y = g.gyro.y;
    if (gyro_z) *gyro_z = g.gyro.z;

    if (magn_x) *magn_x = m.magnetic.x;
    if (magn_y) *magn_y = m.magnetic.y;
    if (magn_z) *magn_z = m.magnetic.z;

    if (acc_x && acc_y && acc_z) imu_to_frame(acc_x,  acc_y,  acc_z);
    if (gyro_x && gyro_y && gyro_z) imu_to_frame(gyro_x, gyro_y, gyro_z);
    if (magn_x && magn_y && magn_z) imu_to_frame(magn_x, magn_y, magn_z);
}

void update_gyro_filter(float gyro_x, float gyro_y, float gyro_z)
{
  const float ALPHA = 0.8f;
  const float BETA = 1.0f - ALPHA;

  _gyro_x = ALPHA * _gyro_x + BETA * gyro_x;
  _gyro_y = ALPHA * _gyro_y + BETA * gyro_y;
  _gyro_z = ALPHA * _gyro_z + BETA * gyro_z;
}


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

    _gyro_x = 0;
    _gyro_y = 0;
    _gyro_z = 0;
    _yaw = 0;
    _pitch = 0;
    _roll = 0;
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
    float acc_x,  acc_y,  acc_z;
    float gyro_x, gyro_y, gyro_z;
    float magn_x, magn_y, magn_z;

    float cur_millis = millis();
    read_imu_raw(&acc_x,  &acc_y,  &acc_z,
                 &gyro_x, &gyro_y, &gyro_z,
                 &magn_x, &magn_y, &magn_z);

    update_gyro_filter(gyro_x, gyro_y, gyro_z);

    gyro_x = _gyro_x - gyro_bias[0];
    gyro_y = _gyro_y - gyro_bias[1];
    gyro_z = _gyro_z - gyro_bias[2];

    float dt_s = 0.1f;
    if (-1 == _lastUpdate_ts)
    {
      attEst.update(dt_s, gyro_x * M_PI/180.0, gyro_y * M_PI/180.0, gyro_z * M_PI/180.0, acc_x, acc_y, acc_z, magn_x, magn_y, magn_z);
      _lastUpdate_ts = cur_millis;

    }
    else
    {
      dt_s = (cur_millis - _lastUpdate_ts) / 1000.0f;
      attEst.update(dt_s, gyro_x * M_PI/180.0, gyro_y * M_PI/180.0, gyro_z * M_PI/180.0, acc_x, acc_y, acc_z, magn_x, magn_y, magn_z);
      _lastUpdate_ts = cur_millis;
    }

    _yaw = attEst.fusedYaw();
    _pitch = attEst.fusedPitch();
    _roll = attEst.fusedRoll();

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
