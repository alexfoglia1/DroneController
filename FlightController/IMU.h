#ifndef IMU_H
#define IMU_H

#define X 0
#define Y 1
#define Z 2

bool IMU_Init();
void IMU_UpdateKFBeta(float beta);
void IMU_Update(float acc[3], float gyro[3], float magn[3], uint64_t* dt);
void IMU_CurrentAttitude(float* roll, float* pitch, float* yaw);
void IMU_CurrentVariance(float* roll, float* pitch, float* yaw);


#endif //IMU_H
