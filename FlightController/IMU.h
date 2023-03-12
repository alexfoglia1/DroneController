#ifndef IMU_H
#define IMU_H

#define IMU_TO_FRAME_ROTATION -0.785398163f //-pi/4

typedef struct
{
  float x;
  float y;
  float z;
} vector3d_t;


bool IMU_Init();
void IMU_Update();
void IMU_UpdateBeta(float beta);
void IMU_CurrentAttitude(float* roll, float* pitch, float* yaw);


#endif //IMU_H
