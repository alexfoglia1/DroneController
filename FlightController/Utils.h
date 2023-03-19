#ifndef UTILS_H
#define UTILS_H

float minMax(float value, float min_value, float max_value);
float toRange(float value, float inMin, float inMax, float outMin, float outMax);
float normalizedPulseIn(int pin, float minPulseWidth, float maxPulseWidth);

#endif //UTILS_H
