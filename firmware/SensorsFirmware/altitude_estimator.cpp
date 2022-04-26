#include "altitude_estimator.h"

#include <Arduino.h>

AltitudeEstimator::AltitudeEstimator(double lpfBeta, double seaLevelPressure)
{
  _lpfBeta = lpfBeta;
  _seaLevelPressure = seaLevelPressure;
}

void AltitudeEstimator::setInitialPressure(double pressure)
{
  _pressure = pressure;
}

double AltitudeEstimator::update(double pressure)
{
  _pressure = _pressure - (_lpfBeta * (_pressure - pressure));
  return 44330.0 * (1.0 - pow(_pressure /_seaLevelPressure, 0.19029495));
}
