class AltitudeEstimator
{
  public:
    AltitudeEstimator(double lpfBeta, double seaLevelPressure);
    void setInitialPressure(double initialPressure);

    double update(double pressure);
    
  private:
    double _lpfBeta;
    double _pressure;
    double _seaLevelPressure;
};
