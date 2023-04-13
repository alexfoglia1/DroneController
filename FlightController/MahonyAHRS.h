#include <math.h>

class MahonyAHRS {

public:
  MahonyAHRS()
  {
    q1 = 1.0f;
    q2 = 0.0f;
    q3 = 0.0f;
    q4 = 0.0f;
    gx = 0.0f;
    gy = 0.0f;
    gz = 0.0f;
    ax = 0.0f;
    ay = 0.0f;
    az = 9.81f;
    mx = 0.0f;
    my = 0.0f;
    mz = 0.0f;
    kp = 1.0f;
    ki = 0.0000;
		integralFBx = 0.0f, integralFBy = 0.0f, integralFBz = 0.0f;
	}
	void mahonyAHRSupdate(float dt, float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz);

	inline float& getQ1()
	{
		return q1;
	}
	inline float& getQ2()
	{
		return q2;
	}
	inline float& getQ3()
	{
		return q3;
	}
	inline float& getQ4()
	{
		return q4;
	}
	inline float& Kp()
	{
		return kp;
	}
	inline float& Ki()
	{
		return ki;
	}
 
  private:
    float q1, q2, q3, q4;
    float gx, gy, gz;
    float ax, ay, az;
    float mx, my, mz;
    float kp, ki;
    float integralFBx, integralFBy, integralFBz;
    float invSqrt(float x);
};
