#ifndef IMU_H
#define IMU_H

void initImu();
void updateImuFilter();
char imuErrorState();
void getImuAttitude(float* roll, float* pitch, float* yaw);
void getImuTemperature(float* temperature);


#endif //IMU_H
