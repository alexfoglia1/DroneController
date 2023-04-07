#ifndef PID_H
#define PID_H

#define ROLL  0
#define PITCH 1
#define YAW   2

extern float PID[3];
extern float KP[3];
extern float KI[3];
extern float KD[3];

void PID_Update(float command[3], float current[3], float dt);

#endif
