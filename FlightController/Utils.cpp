#include <Arduino.h>
#include <stdint.h>

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


float normalizedAnalogRead(int pin)
{
  int16_t adc_in = analogRead(pin);
  return (float)adc_in / 1023.f;
}


void int_to_ascii(int value, char* ascii, int len)
{
  for (int i = 0; i < len - 1; i++)
  {
    ascii[i] = '0';
  }
  ascii[len - 1] = '\0';

  if (value < 0)
  {
    value *= -1;
    ascii[0] = '-';
  }
  else
  {
    ascii[0] = ' ';
  }

  int cur_pos = len - 2;
  while (value && cur_pos > 1)
  {
    uint8_t digit = (uint8_t)(value % 10);
    ascii[cur_pos] = (char)(digit + '0');
    cur_pos -= 1;
    value /= 10;
  }
}
