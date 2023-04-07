#include <math.h>
#define GRAU2RAD 0.01745329251 

float invSqrt(float x);

class MahonyAHRS {
	float q1, q2, q3, q4;
	float kp, ki;
	float integralFBx, integralFBy, integralFBz;
public:
	MahonyAHRS() {
    q1 = 1.0f;
    q2 = 0.0f;
    q3 = 0.0f;
    q4 = 0.0f;
    kp = 12.0f;
    ki = 0.0005;
		integralFBx = 0.0f, integralFBy = 0.0f, integralFBz = 0.0f;
	}
	void mahonyAHRSupdate(float dt, float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz);

	inline float& getQ1() {
		return q1;
	}
	inline float& getQ2() {
		return q2;
	}
	inline float& getQ3() {
		return q3;
	}
	inline float& getQ4() {
		return q4;
	}
	inline float& Kp() {
		return kp;
	}
	inline float& Ki() {
		return ki;
	}
};
