#include "MahonyAHRS.h"
#include <Arduino.h>
//=====================================================================================================
// MahonyAHRS
//=====================================================================================================
//
// Madgwick's implementation of Mayhony's AHRS algorithm.
// See: http://www.x-io.co.uk/node/8#open_source_ahrs_and_imu_algorithms
//
// Date      Author      Notes
// 29/09/2011 SOH Madgwick    Initial release
// 02/10/2011 SOH Madgwick  Optimised for reduced CPU load
// 10/10/2016   JP  Alves       Optimized for reduced CPU load and asynchronous communication (Arduino) 
//=====================================================================================================

void MahonyAHRS::mahonyAHRSupdate(float dt, float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz)
{
  float _norm;
  float hx, hy, bx, bz;
  float vx, vy, vz, wx, wy, wz;
  float ex, ey, ez;
  float pa, pb, pc;

  // Auxiliary variables to avoid repeated arithmetic
  float q1q1 = q1 * q1;
  float q1q2 = q1 * q2;
  float q1q3 = q1 * q3;
  float q1q4 = q1 * q4;
  float q2q2 = q2 * q2;
  float q2q3 = q2 * q3;
  float q2q4 = q2 * q4;
  float q3q3 = q3 * q3;
  float q3q4 = q3 * q4;
  float q4q4 = q4 * q4;   

  // Normalise accelerometer measurement
  _norm = (float)sqrt(ax * ax + ay * ay + az * az);
  if (_norm == 0.f) return; // handle NaN
  _norm = 1 / _norm;        // use reciprocal for division
  ax *= _norm;
  ay *= _norm;
  az *= _norm;

  // Normalise magnetometer measurement
  _norm = (float)sqrt(mx * mx + my * my + mz * mz);
  if (_norm == 0.f) return; // handle NaN
  _norm = 1 / _norm;        // use reciprocal for division
  mx *= _norm;
  my *= _norm;
  mz *= _norm;

  // Reference direction of Earth's magnetic field
  hx = 2.f * mx * (0.5f - q3q3 - q4q4) + 2.f * my * (q2q3 - q1q4) + 2.f * mz * (q2q4 + q1q3);
  hy = 2.f * mx * (q2q3 + q1q4) + 2.f * my * (0.5f - q2q2 - q4q4) + 2.f * mz * (q3q4 - q1q2);
  bx = (float)sqrt((hx * hx) + (hy * hy));
  bz = 2.f * mx * (q2q4 - q1q3) + 2.f * my * (q3q4 + q1q2) + 2.f * mz * (0.5f - q2q2 - q3q3);

  // Estimated direction of gravity and magnetic field
  vx = 2.f * (q2q4 - q1q3);
  vy = 2.f * (q1q2 + q3q4);
  vz = q1q1 - q2q2 - q3q3 + q4q4;
  wx = 2.f * bx * (0.5f - q3q3 - q4q4) + 2.f * bz * (q2q4 - q1q3);
  wy = 2.f * bx * (q2q3 - q1q4) + 2.f * bz * (q1q2 + q3q4);
  wz = 2.f * bx * (q1q3 + q2q4) + 2.f * bz * (0.5f - q2q2 - q3q3);  

  // Error is cross product between estimated direction and measured direction of gravity
  ex = (ay * vz - az * vy) + (my * wz - mz * wy);
  ey = (az * vx - ax * vz) + (mz * wx - mx * wz);
  ez = (ax * vy - ay * vx) + (mx * wy - my * wx);
  if (ki > 0.f)
  {
      integralFBx += ex;      // accumulate integral error
      integralFBy += ey;
      integralFBz += ez;
  }
  else
  {
      integralFBx = 0.0f;     // prevent integral wind up
      integralFBy = 0.0f;
      integralFBz = 0.0f;
  }

  // Apply feedback terms
  gx = gx + kp * ex + ki * integralFBx;
  gy = gy + kp * ey + ki * integralFBy;
  gz = gz + kp * ez + ki * integralFBz;

  // Integrate rate of change of quaternion
  pa = q2;
  pb = q3;
  pc = q4;
  q1 = q1 + (-q2 * gx - q3 * gy - q4 * gz) * (0.5f * dt);
  q2 = pa + (q1 * gx + pb * gz - pc * gy) * (0.5f * dt);
  q3 = pb + (q1 * gy - pa * gz + pc * gx) * (0.5f * dt);
  q4 = pc + (q1 * gz + pa * gy - pb * gx) * (0.5f * dt);

  // Normalise quaternion
  _norm = invSqrt(q1 * q1 + q2 * q2 + q3 * q3 + q4 * q4);

  q1 = q1 * _norm;
  q2 = q2 * _norm;
  q3 = q3 * _norm;
  q4 = q4 * _norm;
}

float invSqrt(float x) {
  volatile float halfx = 0.5f * x;
  volatile float y = x;
  volatile long i = *(long*)&y;
  i = 0x5f3759df - (i>>1);
  y = *(float*)&i;
  for(int i=0;i < 3;i++)  
    y = y * (1.5f - (halfx * y * y));
  return y;
}
