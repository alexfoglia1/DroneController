#include "PID.h"

               //roll,  pitch,  yaw
float KP[3] = {20.00f,  20.00f,  0.00f};
float KI[3] = {0.00f,   0.00f,  0.00f};
float KD[3] = {0.00f,   0.00f,  0.00f};


float int_coef[3] = {00.0f,  00.0f,  00.0f};
float prev_err[3] = {00.0f,  00.0f,  00.0f};
float PID[3]      = {00.0f,  00.0f,  00.0f};


void PID_Update(float command[3], float current[3], float dt)
{
  // (1) -->         ------>          ------>
  //     ERR      =  COMMAND        - CURRENT
  float  error[3] = {command[ROLL]  - current[ROLL],
                     command[PITCH] - current[PITCH],
                     command[YAW]   - current[YAW]};
                     
  // (2) ------->    
  //     INT_COEF =  <Ʃ ERROR_ROLL, Ʃ ERROR_PITCH, Ʃ ERROR_YAW> 
  int_coef[ROLL]     += error[ROLL];
  int_coef[PITCH]    += error[PITCH];
  int_coef[YAW]      += error[YAW];

  // (3) ------>         ---->          ------->
  //     D_ERROR      =  ERROR        - PREV_ERROR
  float  delta_err[3] = {(error[ROLL]  - prev_err[ROLL])/dt,
                         (error[PITCH] - prev_err[PITCH])/dt,
                         (error[YAW]   - prev_err[YAW])/dt};
  
  // (4) ------->   -->
  //     PREV_ERR = ERR
  prev_err[ROLL]  = error[ROLL];
  prev_err[PITCH] = error[PITCH];
  prev_err[YAW]   = error[YAW];

  // (5) --->
  //     PID =  e.KP                        + ∫e.KI                         + Δe.KD
  PID[ROLL]  =  (error[ROLL]  * KP[ROLL])   + (int_coef[ROLL]  * KI[ROLL])  + (delta_err[ROLL]  * KD[ROLL]);
  PID[PITCH] =  (error[PITCH] * KP[PITCH])  + (int_coef[PITCH] * KI[PITCH]) + (delta_err[PITCH] * KD[PITCH]);
  PID[YAW]   =  (error[YAW]   * KP[YAW])    + (int_coef[YAW]   * KI[YAW])   + (delta_err[YAW]   * KD[YAW]);
}


float* PID_Get()
{
  return PID;
}


float* PID_Kp()
{
  return KP;
}


float* PID_Ki()
{
  return KI;
}


float* PID_Kd()
{
  return KD;
}
