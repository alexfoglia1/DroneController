//=============================================================================================
// MadgwickAHRS.h
//=============================================================================================
//
// Implementation of Madgwick's IMU and AHRS algorithms.
// See: http://www.x-io.co.uk/open-source-imu-and-ahrs-algorithms/
//
// From the x-io website "Open-source resources available on this website are
// provided under the GNU General Public Licence unless an alternative licence
// is provided in source."
//
// Date			Author          Notes
// 29/09/2011	SOH Madgwick    Initial release
// 02/10/2011	SOH Madgwick	Optimised for reduced CPU load
// 07/04/2022   Alex Foglia     Removed magnetometer plus added method to modify beta
//
//=============================================================================================
#ifndef MadgwickAHRS_h
#define MadgwickAHRS_h
#include <math.h>

//--------------------------------------------------------------------------------------------
// Variable declaration
class Madgwick{
private:
    static float invSqrt(float x);
    float beta;				// algorithm gain
    float q0;
    float q1;
    float q2;
    float q3;	// quaternion of sensor frame relative to auxiliary frame
    float roll;
    float pitch;
    float yaw;
    char anglesComputed;
    
    void computeAngles();

//-------------------------------------------------------------------------------------------
// Function declarations
public:
    Madgwick(void);
    void update(float dt, float gx, float gy, float gz, float ax, float ay, float az);

    float getRoll() {
        if (!anglesComputed) computeAngles();
        return roll * 57.29578f;
    }
    float getPitch() {
        if (!anglesComputed) computeAngles();
        return pitch * 57.29578f;
    }
    float getYaw() {
        if (!anglesComputed) computeAngles();
        return yaw * 57.29578f + 180.0f;
    }

    void updateBeta(float newBeta)
    {
        if (newBeta < 0.0f)
        {
            newBeta = 0.0f;
        }
        if (newBeta > 1.0f)
        {
            newBeta = 1.0f;
        }

        beta = newBeta;
        q0 = 1.0f;
        q1 = 0.0f;
        q2 = 0.0f;
        q3 = 0.0f;
        anglesComputed = 0;
    }
};
#endif

