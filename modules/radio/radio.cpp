#include "modules/radio/radio.h"
#include "modules/settings/settings.h"

#include <QFile>

RadioDriver::RadioDriver()
{
    Settings s = Settings::instance();
    _serialPortName = s.getAttribute(Settings::Attribute::RADIO_DEVICE).toString();
    _baudRate = s.getAttribute(Settings::Attribute::RADIO_BAUD).toInt();
    _serialPort = nullptr;
}

bool RadioDriver::init()
{
    if (!QFile(_serialPortName).exists())
    {
        return false;
    }
    else
    {
        _serialPort = new QSerialPort(_serialPortName);
        _serialPort->setBaudRate(_baudRate);

        connect(_serialPort, &QSerialPort::readyRead, this, &RadioDriver::receiveData);
        connect(_serialPort, &QSerialPort::errorOccurred, this, &RadioDriver::handleError);

        return true;
    }
}

#include <iostream>
void RadioDriver::receiveData()
{
    std::cout << "Received something...\n";
}

void RadioDriver::handleError()
{
    std::cout << "An error occurred...\n";
}

void RadioDriver::downlink()
{
    std::cout << "Se stessi facendo il leoss alzerei un bit porca madonna\n";
}
