#include <Arduino.h>


float minMax(float value, float min_value, float max_value)
{
    if (value > max_value)
    {
        value = max_value;
    }
    else if (value < min_value)
    {
        value = min_value;
    }

    return value;
}


float toRange(float value, float inMin, float inMax, float outMin, float outMax)
{
    value = minMax(value, inMin, inMax);
    float rangeIn = (inMax - inMin);
    float rangeOut = (outMax - outMin);

    float inPercentage = (value - inMin) / rangeIn;
    return outMin + inPercentage * rangeOut;
}


float normalizedPulseIn(int pin, float minPulseWidth, float maxPulseWidth)
{
    int pulseWidth = pulseIn(pin, HIGH);
    return toRange(pulseWidth, minPulseWidth, maxPulseWidth, 0.f, 1.f);
}
