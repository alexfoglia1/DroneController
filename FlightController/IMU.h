#ifndef IMU_H
#define IMU_H

#define X 0
#define Y 1
#define Z 2

bool IMU_Init(float sampling_period_s);
void IMU_EstimateBias(int N);
void IMU_EnableMovingAVGFilter();
void IMU_DisableMovingAVGFilter();
void IMU_Update(float acc[3], float gyro[3], float magn[3]);
void IMU_CurrentAttitude(float* roll, float* pitch, float* yaw);


#endif //IMU_H
