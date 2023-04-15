#ifndef UTILS_H
#define UTILS_H

float minMax(float value, float min_value, float max_value);
float toRange(float value, float inMin, float inMax, float outMin, float outMax);
float normalizedPulseIn(int pin, float minPulseWidth, float maxPulseWidth);
float normalizedAnalogRead(int pin);
float roundDecimal(float f, int nDigits);
void  int_to_ascii(int value, char* ascii, int len);

#endif //UTILS_H
