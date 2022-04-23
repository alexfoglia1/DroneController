#ifndef ULTRASONIC_H
#define ULTRASONIC_H

class UltraSonic
{
  public:
    UltraSonic(int echoPin, int trigPin);
    float distance();

  private:
    int _echoPin;
    int _trigPin;

    float _d0;
};

#endif //ULTRASONIC_H
