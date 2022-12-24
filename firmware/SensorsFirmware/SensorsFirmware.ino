#include "altitude_estimator.h"
#include "attitude_estimator.h"
#include "proto.h"

/** Baro library **/
#include <Arduino_LPS22HB.h>
/** IMU library **/
#include <Arduino_LSM9DS1.h>

const double LPF_BETA = 0.05;
const double SL_PRESSURE_KPA = 101.5; // Should be configurable: Measurement of Florence Airport
const double DELAY_MILLIS = 10;

char serialRxBuffer[512];
int numChars = 512;

AltitudeEstimator altEst(LPF_BETA, SL_PRESSURE_KPA);
stateestimation::AttitudeEstimator attEst;

int lastUpdate_ts;
uint8_t errorCode;
BleToUnoMessage msgOut;
BleToUnoErrorMessage eMsgOut;

void setup()
{
  Serial.begin(9600);
  
  lastUpdate_ts = -1;
  errorCode = NO_ERRORS;

  if (!BARO.begin())
  {
    errorCode |= BARO_NO_SUCH_DEVICE;
  }
  else
  {
    altEst.setInitialPressure(BARO.readPressure());
  }

  if (!IMU.begin())
  {
    errorCode |= IMU_NO_SUCH_DEVICE;
  }
  else
  {
    attEst.setMagCalib(0.68, -1.32, 0.0);
    attEst.setPIGains(2.2, 2.65, 10, 1.25); 
  }

  msgOut.msg_id = BLE_TO_UNO_MSG_ID;
  msgOut.pitch = 0.f;
  msgOut.roll = 0.f;
  msgOut.yaw = 0.f;
  msgOut.baro_altitude = 0.f;
  
  eMsgOut.msg_id = BLE_TO_UNO_EMSG_ID;
  eMsgOut.error_code = errorCode;
}


void loop()
{
  if (NO_ERRORS == errorCode)
  {
    if (IMU.gyroscopeAvailable() && IMU.accelerationAvailable() && IMU.magneticFieldAvailable())
    {
      float gyrox, gyroy, gyroz;
      float accx, accy, accz;
      float magnx, magny, magnz;
      IMU.readGyroscope(gyrox, gyroy, gyroz);
      IMU.readAcceleration(accx, accy, accz);
      IMU.readMagneticField(magnx, magny, magnz);
      if (-1 == lastUpdate_ts)
      {
        double dt_s = DELAY_MILLIS / 1000.0;
        attEst.update(dt_s, gyrox * M_PI/180.0, gyroy * M_PI/180.0, gyroz * M_PI/180.0, accx, accy, accz, magnx, magny, magnz);
        lastUpdate_ts = millis();
      }
      else
      {
        int t = millis();
        double dt_s = (t - lastUpdate_ts) / 1000.0;
        attEst.update(dt_s, gyrox * M_PI/180.0, gyroy * M_PI/180.0, gyroz * M_PI/180.0, accx, accy, accz, magnx, magny, magnz);
        lastUpdate_ts = t;
      }
    }
  
    double altitude = altEst.update(BARO.readPressure());
    msgOut.pitch = (float)(attEst.fusedPitch());
    msgOut.roll = (float)(attEst.fusedRoll());
    msgOut.yaw = (float)(attEst.fusedYaw());
    msgOut.baro_altitude = (float)(altitude);

    txToSerial((char*)&msgOut, sizeof(BleToUnoMessage));
  }
  else
  {
    txToSerial((char*)&eMsgOut, sizeof(BleToUnoErrorMessage));
  }
}

boolean recvFromSerial()
{
    static boolean recvInProgress = false;
    static byte ndx = 0;
    boolean dataFromSerial = false;
    char rc;
 
    while (Serial.available() > 0)
    {
        rc = Serial.read();

        if (recvInProgress == true)
        {
            if (rc != endMarker)
            {
                serialRxBuffer[ndx] = rc;
                ndx++;
                if (ndx >= numChars)
                {
                    ndx = numChars - 1;
                }
            }
            else
            {
                recvInProgress = false;
                dataFromSerial = true;
                ndx = 0;
            }
        }

        else if (rc == startMarker)
        {
            recvInProgress = true;
        }
    }

    return dataFromSerial;
}

void txToSerial(char* data, int len)
{
  char txBuffer[len + 2];
  txBuffer[0] = startMarker;
  for (int i = 0; i < len; i++)
  {
    txBuffer[i + 1] = data[i];
  }
  txBuffer[len + 1] = endMarker;

  Serial.write(txBuffer, len + 2);
  delay((int)(DELAY_MILLIS));
}
