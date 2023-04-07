/***************************************************************************
  This is a library for the LSM9DS1 Accelerometer and magnentometer/compass

  Designed specifically to work with the Adafruit LSM9DS1 Breakouts

  These sensors use I2C to communicate, 2 pins are required to interface.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!

  Written by Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution

  Slightly modified by Alex Foglia for custom projects, Florence (IT)
 ***************************************************************************/
#include "Adafruit_LSM9DS1.h"

/***************************************************************************
 CONSTRUCTOR
 ***************************************************************************/

/**************************************************************************/
/*!
    @brief Instantiate with hardware I2C interface
    @param wireBus The I2C wire interface you'd like to use
    @param sensorID Unique identifier you'd like for the sensors
*/
/**************************************************************************/
Adafruit_LSM9DS1::Adafruit_LSM9DS1(TwoWire *wireBus, int32_t sensorID) {
  _wire = wireBus;
  initSensor(sensorID);
}

/**************************************************************************/
/*!
    @brief Instantiate with default hardware I2C interface
    @param sensorID Unique identifier you'd like for the sensors
*/
/**************************************************************************/
Adafruit_LSM9DS1::Adafruit_LSM9DS1(int32_t sensorID) {
  _wire = &Wire;
  initSensor(sensorID);
}

/**************************************************************************/
/*!
    @brief Instantiate with hardware SPI interface
    @param xgcs SPI CS pin for accelerometer/gyro subchip
    @param sensorID Unique identifier you'd like for the sensors
*/
/**************************************************************************/
Adafruit_LSM9DS1::Adafruit_LSM9DS1(int8_t xgcs, int32_t sensorID) {
  // hardware SPI!
  _csxg = xgcs;
  _mosi = _miso = _clk = -1;
  initSensor(sensorID);
}

/**************************************************************************/
/*!
    @brief Instantiate with software SPI interface
    @param sclk SPI clock pin
    @param smiso SPI MISO pin
    @param smosi SPI MOSI pin
    @param xgcs SPI CS pin for accelerometer/gyro subchip
    @param sensorID Unique identifier you'd like for the sensors
*/
/**************************************************************************/
Adafruit_LSM9DS1::Adafruit_LSM9DS1(int8_t sclk, int8_t smiso, int8_t smosi,
                                   int8_t xgcs, int32_t sensorID) {
  _csxg = xgcs;
  _mosi = smosi;
  _miso = smiso;
  _clk = sclk;
  initSensor(sensorID);
}

/**************************************************************************/
/*!
    @brief Initialize I2C or SPI and detect/initialize subsensors
    @returns True if both subsensors were detected on the desired interface
*/
/**************************************************************************/
bool Adafruit_LSM9DS1::begin() {
  if (_wire) {
    // I2C
    if (i2c_dev)
      delete i2c_dev;
    i2c_dev = new Adafruit_I2CDevice(LSM9DS1_ADDRESS_ACCELGYRO, _wire);
    if (!i2c_dev->begin())
    {
        return false;
    }
  } else {
    // SPI
    if (spi_dev)
      delete spi_dev;
    if (_clk == -1) {
      // Hardware SPI
      spi_dev = new Adafruit_SPIDevice(_csxg);
    } else {
      // Software SPI
      spi_dev = new Adafruit_SPIDevice(_csxg, _clk, _miso, _mosi);
    }
    if (!spi_dev->begin())
      return false;
  }

  // soft reset & reboot accel/gyro
  write8(XGTYPE, LSM9DS1_REGISTER_CTRL_REG8, 0x05);

  delay(10);

  uint8_t id = read8(XGTYPE, LSM9DS1_REGISTER_WHO_AM_I_XG);
  if (id != LSM9DS1_XG_ID)
    return false;

  // enable gyro continuous
  write8(XGTYPE, LSM9DS1_REGISTER_CTRL_REG1_G, 0xC0); // on XYZ

  // Enable the accelerometer continous
  write8(XGTYPE, LSM9DS1_REGISTER_CTRL_REG5_XL, 0x38); // enable X Y and Z axis
  write8(XGTYPE, LSM9DS1_REGISTER_CTRL_REG6_XL,
         0xC0); // 1 KHz out data rate, BW set by ODR, 408Hz anti-aliasing

  // Set default ranges for the various sensors
  setupAccel(LSM9DS1_ACCELRANGE_2G);
  setupGyro(LSM9DS1_GYROSCALE_245DPS);

  return true;
}

/***************************************************************************
 PUBLIC FUNCTIONS
 ***************************************************************************/

/**************************************************************************/
/*!
    @brief Read all four sensor subcomponents
*/
/**************************************************************************/
void Adafruit_LSM9DS1::read() {
  /* Read all the sensors. */
  readAccel();
  readGyro();
}

/**************************************************************************/
/*!
    @brief Read the sensor accelerometer sensor component
*/
/**************************************************************************/
void Adafruit_LSM9DS1::readAccel() {
  // Read the accelerometer
  byte buffer[6];
  readBuffer(XGTYPE, 0x80 | LSM9DS1_REGISTER_OUT_X_L_XL, 6, buffer);

  uint8_t xlo = buffer[0];
  int16_t xhi = buffer[1];
  uint8_t ylo = buffer[2];
  int16_t yhi = buffer[3];
  uint8_t zlo = buffer[4];
  int16_t zhi = buffer[5];

  // Shift values to create properly formed integer (low byte first)
  xhi <<= 8;
  xhi |= xlo;
  yhi <<= 8;
  yhi |= ylo;
  zhi <<= 8;
  zhi |= zlo;
  accelData.x = xhi;
  accelData.y = yhi;
  accelData.z = zhi;
}

/**************************************************************************/
/*!
    @brief Read the sensor gyroscope sensor component
*/
/**************************************************************************/
void Adafruit_LSM9DS1::readGyro() {
  // Read gyro
  byte buffer[6];
  readBuffer(XGTYPE, 0x80 | LSM9DS1_REGISTER_OUT_X_L_G, 6, buffer);

  uint8_t xlo = buffer[0];
  int16_t xhi = buffer[1];
  uint8_t ylo = buffer[2];
  int16_t yhi = buffer[3];
  uint8_t zlo = buffer[4];
  int16_t zhi = buffer[5];

  // Shift values to create properly formed integer (low byte first)
  xhi <<= 8;
  xhi |= xlo;
  yhi <<= 8;
  yhi |= ylo;
  zhi <<= 8;
  zhi |= zlo;

  gyroData.x = xhi;
  gyroData.y = yhi;
  gyroData.z = zhi;
}

/**************************************************************************/
/*!
    @brief Configure the accelerometer ranging
    @param range Can be LSM9DS1_ACCELRANGE_2G, LSM9DS1_ACCELRANGE_4G,
    LSM9DS1_ACCELRANGE_8G, LSM9DS1_ACCELRANGE_16G
*/
/**************************************************************************/
void Adafruit_LSM9DS1::setupAccel(lsm9ds1AccelRange_t range) {
  uint8_t reg = read8(XGTYPE, LSM9DS1_REGISTER_CTRL_REG6_XL);
  reg &= ~(0b00011000);
  reg |= range;
  // Serial.println("set range: ");
  write8(XGTYPE, LSM9DS1_REGISTER_CTRL_REG6_XL, reg);

  switch (range) {
  case LSM9DS1_ACCELRANGE_2G:
    _accel_mg_lsb = LSM9DS1_ACCEL_MG_LSB_2G;
    break;
  case LSM9DS1_ACCELRANGE_4G:
    _accel_mg_lsb = LSM9DS1_ACCEL_MG_LSB_4G;
    break;
  case LSM9DS1_ACCELRANGE_8G:
    _accel_mg_lsb = LSM9DS1_ACCEL_MG_LSB_8G;
    break;
  case LSM9DS1_ACCELRANGE_16G:
    _accel_mg_lsb = LSM9DS1_ACCEL_MG_LSB_16G;
    break;
  }
}

/**************************************************************************/
/*!
    @brief Configure the gyroscope scaling
    @param scale Can be LSM9DS1_GYROSCALE_245DPS, LSM9DS1_GYROSCALE_500DPS
    or LSM9DS1_GYROSCALE_2000DPS
*/
/**************************************************************************/
void Adafruit_LSM9DS1::setupGyro(lsm9ds1GyroScale_t scale) {
  uint8_t reg = read8(XGTYPE, LSM9DS1_REGISTER_CTRL_REG1_G);
  reg &= ~(0b00011000);
  reg |= scale;
  write8(XGTYPE, LSM9DS1_REGISTER_CTRL_REG1_G, reg);

  switch (scale) {
  case LSM9DS1_GYROSCALE_245DPS:
    _gyro_dps_digit = LSM9DS1_GYRO_DPS_DIGIT_245DPS;
    break;
  case LSM9DS1_GYROSCALE_500DPS:
    _gyro_dps_digit = LSM9DS1_GYRO_DPS_DIGIT_500DPS;
    break;
  case LSM9DS1_GYROSCALE_2000DPS:
    _gyro_dps_digit = LSM9DS1_GYRO_DPS_DIGIT_2000DPS;
    break;
  }
}

/***************************************************************************
 UNIFIED SENSOR FUNCTIONS
 ***************************************************************************/

/**************************************************************************/
/*!
    @brief  Gets the most recent accel sensor events for all 4 sensors
    @param accelEvent The accelerometer event object we will fill, pass NULL to
   skip
    @param gyroEvent The gyroscope event object we will fill, pass NULL to skip
    @returns True on successful reads
*/
/**************************************************************************/
bool Adafruit_LSM9DS1::getEvent(sensors_event_t *accelEvent,
                                sensors_event_t *gyroEvent) {
  /* Grab new sensor reading and timestamp. */
  read();
  uint32_t timestamp = millis();

  /* Update appropriate sensor events. */
  if (accelEvent)
    getAccelEvent(accelEvent, timestamp);
  if (gyroEvent)
    getGyroEvent(gyroEvent, timestamp);

  return true;
}

/**************************************************************************/
/*!
    @brief Gets the sensor_t data for all 4 sub-sensors at once call
    @param accel The accelerometer sensor_t object we will fill, pass NULL to
   skip
    @param gyro The gyroscope sensor_t object we will fill, pass NULL to skip
*/
/**************************************************************************/
void Adafruit_LSM9DS1::getSensor(sensor_t *accel, sensor_t *gyro) {
  /* Update appropriate sensor metadata. */
  if (accel)
    getAccelSensor(accel);
  if (gyro)
    getGyroSensor(gyro);

}

/**************************************************************************/
/*!
    @brief Fill in the details about the most recent accelerometer data read
    @param event The sensor_event_t object we will fill!
    @param timestamp Unused
*/
/**************************************************************************/
void Adafruit_LSM9DS1::getAccelEvent(sensors_event_t *event,
                                     uint32_t timestamp) {
  memset(event, 0, sizeof(sensors_event_t));
  event->version = sizeof(sensors_event_t);
  event->sensor_id = _lsm9dso_sensorid_accel;
  event->type = SENSOR_TYPE_ACCELEROMETER;
  event->timestamp = timestamp;
  event->acceleration.x = accelData.x * _accel_mg_lsb;
  event->acceleration.x /= 1000;
  event->acceleration.x *= SENSORS_GRAVITY_STANDARD;
  event->acceleration.y = accelData.y * _accel_mg_lsb;
  event->acceleration.y /= 1000;
  event->acceleration.y *= SENSORS_GRAVITY_STANDARD;
  event->acceleration.z = accelData.z * _accel_mg_lsb;
  event->acceleration.z /= 1000;
  event->acceleration.z *= SENSORS_GRAVITY_STANDARD;
}

/**************************************************************************/
/*!
    @brief Fill in the details about the most recent gyroscope data read
    @param event The sensor_event_t object we will fill!
    @param timestamp The millis timestamp when the read occured
*/
/**************************************************************************/
void Adafruit_LSM9DS1::getGyroEvent(sensors_event_t *event,
                                    uint32_t timestamp) {
  memset(event, 0, sizeof(sensors_event_t));
  event->version = sizeof(sensors_event_t);
  event->sensor_id = _lsm9dso_sensorid_accel;
  event->type = SENSOR_TYPE_GYROSCOPE;
  event->timestamp = timestamp;
  event->gyro.x = gyroData.x * _gyro_dps_digit * SENSORS_DPS_TO_RADS;
  event->gyro.y = gyroData.y * _gyro_dps_digit * SENSORS_DPS_TO_RADS;
  event->gyro.z = gyroData.z * _gyro_dps_digit * SENSORS_DPS_TO_RADS;
}

/**************************************************************************/
/*!
    @brief Fill in the details about the accelerometer sensor component
    @param sensor The sensor_t object we will fill!
*/
/**************************************************************************/
void Adafruit_LSM9DS1::getAccelSensor(sensor_t *sensor) {
  memset(sensor, 0, sizeof(sensor_t));
  strncpy(sensor->name, "LSM9DS1_A", sizeof(sensor->name) - 1);
  sensor->name[sizeof(sensor->name) - 1] = 0;
  sensor->version = 1;
  sensor->sensor_id = _lsm9dso_sensorid_accel;
  sensor->type = SENSOR_TYPE_ACCELEROMETER;
  sensor->min_delay = 0;
  sensor->max_value = 156.8;      // +16 g = 156.8 m/s^s
  sensor->min_value = -156.8;     // -16 g = 156.8 m/s^s
  sensor->resolution = 0.0005978; // 0.061 mg = 0.0005978 m/s^2
}

/**************************************************************************/
/*!
    @brief Fill in the details about the gyroscope sensor component
    @param sensor The sensor_t object we will fill!
*/
/**************************************************************************/
void Adafruit_LSM9DS1::getGyroSensor(sensor_t *sensor) {
  memset(sensor, 0, sizeof(sensor_t));
  strncpy(sensor->name, "LSM9DS1_G", sizeof(sensor->name) - 1);
  sensor->name[sizeof(sensor->name) - 1] = 0;
  sensor->version = 1;
  sensor->sensor_id = _lsm9dso_sensorid_gyro;
  sensor->type = SENSOR_TYPE_GYROSCOPE;
  sensor->min_delay = 0;
  sensor->max_value = 34.91;             // +2000 dps = 34.906586 rad/s
  sensor->min_value = -34.91;            // "
  sensor->resolution = 0.00015271631375; // 8.75 mdps = 0.00015271631375 rad/s
}


/***************************************************************************
 PRIVATE FUNCTIONS
 ***************************************************************************/
void Adafruit_LSM9DS1::initSensor(int32_t sensorID) {
  _lsm9dso_sensorid_accel = sensorID + 1;
  _lsm9dso_sensorid_gyro = sensorID + 3;
  _accelSensor = Sensor(this, &Adafruit_LSM9DS1::readAccel,
                        &Adafruit_LSM9DS1::getAccelEvent,
                        &Adafruit_LSM9DS1::getAccelSensor);
  _gyroSensor =
      Sensor(this, &Adafruit_LSM9DS1::readGyro, &Adafruit_LSM9DS1::getGyroEvent,
             &Adafruit_LSM9DS1::getGyroSensor);
}

void Adafruit_LSM9DS1::write8(boolean type, byte reg, byte value) {
  uint8_t buffer[2] = {i2c_dev ? uint8_t(reg) : uint8_t(reg & 0x7F), value};
  if (i2c_dev) {
    i2c_dev->write(buffer, 2);
  } else {
    spi_dev->write(buffer, 2);
  }
}

byte Adafruit_LSM9DS1::read8(boolean type, byte reg) {
  uint8_t value;

  readBuffer(type, reg, 1, &value);

  return value;
}

byte Adafruit_LSM9DS1::readBuffer(boolean type, byte reg, byte len,
                                  uint8_t *buffer) {

  uint8_t regbuf[1] = {i2c_dev ? uint8_t(reg) : uint8_t(reg | 0x80)};
  if (i2c_dev) {
    i2c_dev->write_then_read(regbuf, 1, buffer, len);
  } else {
    spi_dev->write_then_read(regbuf, 1, buffer, len);
  }

  return len;
}
