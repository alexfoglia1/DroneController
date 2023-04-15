#ifndef PID_H
#define PID_H

#define ROLL  0
#define PITCH 1
#define YAW   2

void PID_Update(float setpoint[3], float feedback[3], float dt);
float* PID_Get();
float* PID_Kp();
float* PID_Ki();
float* PID_Kd();

#endif
