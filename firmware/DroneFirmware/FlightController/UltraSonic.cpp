#include "UltraSonic.h"
#include <Arduino.h>

UltraSonic::UltraSonic(int echoPin, int trigPin)
{
  _echoPin = echoPin;
  _trigPin = trigPin;

  pinMode(_echoPin, INPUT);
  pinMode(_trigPin, OUTPUT);
}

float UltraSonic::distance()
{
  digitalWrite(_trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(_trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(_trigPin, LOW);
  int duration = pulseIn(_echoPin, HIGH);
  float distance = duration * 0.034f / 2.0f;

  return distance;
}
