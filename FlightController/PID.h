#ifndef PID_H
#define PID_H

#define ROLL  0
#define PITCH 1
#define YAW   2

extern float PID[3];
void PID_Update(float command[3], float current[3]);

#endif
