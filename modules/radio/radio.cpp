#include "modules/radio/radio.h"
#include "modules/settings/settings.h"

#include <QtEndian>

RadioDriver::RadioDriver()
{
    _state = OFF;

    Settings* s = Settings::instance();

    _serialPortName = s->getAttribute(Settings::Attribute::RADIO_DEVICE).toString();
    _baudRate = s->getAttribute(Settings::Attribute::RADIO_BAUD).toInt();
    _txTimeoutMillis = 1000.0 / s->getAttribute(Settings::Attribute::RADIO_TX_FREQ).toInt();
    _rxTimeoutMillis = 1000.0;

    _serialPort = nullptr;
    _gotStart = false;
    _gotEnd = false;

    _configMsg.msg_id = CTRL_TO_RADIO_CFG_ID;
    _configMsg.rx_pipe = s->getAttribute(Settings::Attribute::RADIO_RX_PIPE).toULongLong();
    _configMsg.tx_pipe = s->getAttribute(Settings::Attribute::RADIO_TX_PIPE).toULongLong();

    _commandMsg.msg_id = CTRL_TO_RADIO_CMD_ID;
    _commandMsg.l2_axis = 0x00;
    _commandMsg.r2_axis = 0x00;
    _commandMsg.l3_x_axis = 0x00;
    _commandMsg.l3_y_axis = 0x00;
    _commandMsg.r3_x_axis = 0x00;
    _commandMsg.r3_y_axis = 0x00;
}

bool RadioDriver::init()
{
    try
    {
        _serialPort = new QSerialPort();
        _serialPort->setPortName(_serialPortName);
        _serialPort->open(QIODevice::OpenModeFlag::ReadWrite);
        if (!_serialPort->isOpen())
        {
            return false;
        }

        _serialPort->setBaudRate(_baudRate);
        _serialPort->setDataBits(QSerialPort::Data8);
        _serialPort->setParity(QSerialPort::NoParity);
        _serialPort->setStopBits(QSerialPort::OneStop);
        _serialPort->setFlowControl(QSerialPort::NoFlowControl);

        connect(_serialPort, &QSerialPort::readyRead, this, &RadioDriver::receiveData);
        connect(_serialPort, &QSerialPort::errorOccurred, this, &RadioDriver::handleError);

        _txTimer = new QTimer();
        _txTimer->setInterval(_txTimeoutMillis);

        _downlinkTimer = new QTimer();
        _downlinkTimer->setInterval(_rxTimeoutMillis);

        connect(_txTimer, &QTimer::timeout, this, &RadioDriver::transmitData);
        connect(_downlinkTimer, &QTimer::timeout, this, &RadioDriver::downlink);
        return true;
    }
    catch (std::exception)
    {
        return false;
    }
}

void RadioDriver::receiveData()
{
    QByteArray chunk;
    chunk = _serialPort->readAll();
    if (!_gotStart)
    {
        _rxBuffer.clear();
        _gotStart = START_MARKER == chunk.at(0);
        if (_gotStart)
        {
            chunk.remove(0, 1);
            saveChunk(chunk);
        }
    }
    else
    {
        if (!_gotEnd)
        {
            saveChunk(chunk);
        }

        if (_gotEnd)
        {
            dataIngest();

            _rxBuffer.clear();
            _gotStart = false;
            _gotEnd = false;
        }
    }
}

void RadioDriver::handleError(QSerialPort::SerialPortError error)
{
    Q_UNUSED(error);

    _state = OFF;
    emit radioChangedState(OFF);
}

void RadioDriver::downlink()
{
    _state = OFF;
    emit radioChangedState(OFF);
}

void RadioDriver::transmitData()
{
    switch (_state)
    {
        case OFF:
        clearTxBuffer();
        break;
        case NOT_CONFIGURED:
        setupTxBuffer((char*)&_configMsg, sizeof(_configMsg));
        break;
        case RUNNING:
        setupTxBuffer((char*)&_commandMsg, sizeof(_commandMsg));
        break;
    }

    _serialPort->write(_txBuffer);
}

void RadioDriver::dataIngest()
{
    _downlinkTimer->stop();
    uint32_t msgId = *reinterpret_cast<uint32_t*>(_rxBuffer.data());
    switch (msgId)
    {
        case RADIO_TO_CTRL_ALIVE_ID:
        {
            RadioToCtrlAliveMessage msgParsed = *reinterpret_cast<RadioToCtrlAliveMessage*>(_rxBuffer.data());
            receivedRadioAlive(msgParsed);
            break;
        }
        case RADIO_TO_CTRL_ACK_ID:
        {
            RadioToCtrlAckMessage msgParsed = *reinterpret_cast<RadioToCtrlAckMessage*>(_rxBuffer.data());
            receivedRadioAck(msgParsed);
            break;
        }
        default:
            break;

    }
    _downlinkTimer->start();
}

void RadioDriver::receivedRadioAlive(RadioToCtrlAliveMessage msgParsed)
{
    if (_state == OFF)
    {
        emit radioFirmwareVersion(QString("%1.%2-%3").arg(msgParsed.major_v)
                                              .arg(msgParsed.minor_v)
                                              .arg(msgParsed.stage_v).toUpper());


        _state = NOT_CONFIGURED;
        emit radioChangedState(NOT_CONFIGURED);

        _txTimer->start();
        _downlinkTimer->start();

    }
}

void RadioDriver::receivedRadioAck(RadioToCtrlAckMessage msgParsed)
{
    switch (msgParsed.msg_acked)
    {
    case CTRL_TO_RADIO_CFG_ID:
    {
        if (_state == NOT_CONFIGURED)
        {
            _state = RUNNING;
            emit radioChangedState(RUNNING);
        }
    }
    break;
    case CTRL_TO_RADIO_CMD_ID:
    {
        /** Do nothing **/
    }
    break;
    default:
    break;
    }
}

void RadioDriver::saveChunk(QByteArray chunk)
{
    for (int i = 0; i < chunk.size(); i++)
    {
        if (chunk.at(i) == END_MARKER)
        {
            _gotEnd = true;
        }
        else if (!_gotEnd)
        {
            _rxBuffer.push_back(chunk.at(i));
        }
    }
}

void RadioDriver::clearTxBuffer()
{
    _txBuffer.clear();
}

void RadioDriver::setupTxBuffer(char* data, quint64 size)
{
    _txBuffer.clear();
    _txBuffer.push_back(START_MARKER);
    for (quint64 i = 0; i < size; i++)
    {
        _txBuffer.push_back(data[i]);
    }
    _txBuffer.push_back(END_MARKER);
}
