#include "modules/radio/radio.h"

#include <QtEndian>

RadioDriver::RadioDriver()
{
    _state = OFF;

    _serialPort = nullptr;
    _txTimer = nullptr;
    _downlinkTimer = nullptr;
    _gotStart = false;
    _gotEnd = false;

    _confirmButton = 0;
    _baudRate = 0;
    _txTimeoutMillis = 0;
    _rxTimeoutMillis = 0;

#ifdef WIN32
    _serialPortName = "COM1";
#else
    _serialPortName = "ttyUSB0";
#endif

    _configMsg.msg_id = CTRL_TO_RADIO_CFG_ID;
    _configMsg.rx_pipe = 0;
    _configMsg.tx_pipe = 0;
    _configMsg.config_ok = 0;

    _commandMsg.msg_id = CTRL_TO_RADIO_CMD_ID;
    _commandMsg.l2_axis = 0x00;
    _commandMsg.r2_axis = 0x00;
    _commandMsg.l3_x_axis = 0x00;
    _commandMsg.l3_y_axis = 0x00;
    _commandMsg.r3_x_axis = 0x00;
    _commandMsg.r3_y_axis = 0x00;
}

void RadioDriver::applySettings(Settings* settings)
{
    _serialPortName = settings->getAttribute(Settings::Attribute::RADIO_DEVICE).toString();
    _baudRate = settings->getAttribute(Settings::Attribute::RADIO_BAUD).toInt();
    _txTimeoutMillis = 1000.0 / settings->getAttribute(Settings::Attribute::RADIO_TX_FREQ).toInt();
    _rxTimeoutMillis = 1000.0;

    _configMsg.rx_pipe = settings->getAttribute(Settings::Attribute::RADIO_RX_PIPE).toULongLong();
    _configMsg.tx_pipe = settings->getAttribute(Settings::Attribute::RADIO_TX_PIPE).toULongLong();

    _confirmButton = settings->getAttribute(Settings::Attribute::JOYSTICK_BTN_PS).toInt();
}

bool RadioDriver::init()
{
    _state = INIT;
    _txBuffer.clear();
    _rxBuffer.clear();

    emit radioChangedState(INIT);

    if (_serialPort)
    {
        _serialPort->close();
        delete _serialPort;
        _serialPort = nullptr;
    }

    if (_txTimer)
    {
        _txTimer->stop();
        delete _txTimer;
        _txTimer = nullptr;
    }

    if (_downlinkTimer)
    {
        _downlinkTimer->stop();
        delete _downlinkTimer;
        _downlinkTimer = nullptr;
    }

    try
    {
        _serialPort = new QSerialPort();
        _serialPort->setPortName(_serialPortName);
        _serialPort->open(QIODevice::OpenModeFlag::ReadWrite);
        if (!_serialPort->isOpen())
        {
            perror("Serial port not opened");
            delete _serialPort;
            _serialPort = nullptr;
            _state = OFF;
            emit radioChangedState(OFF);

            return false;
        }

        _serialPort->setBaudRate(_baudRate);
        _serialPort->setDataBits(QSerialPort::Data8);
        _serialPort->setParity(QSerialPort::NoParity);
        _serialPort->setStopBits(QSerialPort::OneStop);
        _serialPort->setFlowControl(QSerialPort::NoFlowControl);

        connect(_serialPort, &QSerialPort::readyRead, this, &RadioDriver::receiveData);

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
        /** Ho un chunk, ma non ho ancora letto lo startMarker **/
        _rxBuffer.clear();
        _gotStart = (START_MARKER == chunk.at(0));

        if (_gotStart)
        {
            /** Ho un chunk e lo startMarker, salvo il chunk nel buffer di ricezione **/
            chunk.remove(0, 1);
            _gotEnd = saveChunk(chunk);

            if (_gotEnd)
            {
                /** In un unico chunk ho sia start che end, ottimo! **/
                dataIngest();

                _rxBuffer.clear();
                _gotStart = false;
                _gotEnd = false;
                
            }
        }
        else
        {
            /** Ho ricevuto un messaggio senza startMarker quando me lo aspettavo:
             *  Lo scarto **/
        }
    }
    else if (_gotStart && !_gotEnd)
    {
        /** Ho un chunk, ho già ricevuto lo startMarker ma mi manca l'end:
         *  Accodo il chunk e controllo se contiene endMarker **/

        _gotEnd = saveChunk(chunk);

        if (_gotEnd)
        {
            /** Questo chunk è buono perchè completa il buffer di ricezione con un endMarker
             *  quando ho già ricevuto lo startMarker: ottimo! **/
            dataIngest();

            _rxBuffer.clear();
            _gotStart = false;
            _gotEnd = false;
        }
    }
}

void RadioDriver::downlink()
{
    _state = OFF;
    emit radioChangedState(OFF);
}

void RadioDriver::transmitData()
{
    bool sendFlag = false;
    switch (_state)
    {
        case OFF:
            clearTxBuffer();
            break;
        case INIT:
        case CONFIG_MISMATCH:
        case TO_RUNNING:
        setupTxBuffer((char*)&_configMsg, sizeof(_configMsg));
        sendFlag = true;
        break;
        default:
        setupTxBuffer((char*)&_commandMsg, sizeof(_commandMsg));
        sendFlag = true;
        break;
    }

    if (sendFlag)
    {
        _serialPort->write(_txBuffer);
        _serialPort->waitForBytesWritten(_txTimeoutMillis);
    }

    if (_state == TO_RUNNING)
    {
        _state = RUNNING;
        emit radioChangedState(RUNNING);
    }
}

void RadioDriver::onJsBtnPressed(int btnPressed)
{
    if (btnPressed == _confirmButton)
    {
        init();
    }
}

void RadioDriver::onJsMessageUpdate(CtrlToRadioCommandMessage msgOut)
{
    _commandMsg = msgOut;
}

void RadioDriver::dataIngest()
{
    _downlinkTimer->stop();
    uint8_t msgId = *reinterpret_cast<uint8_t*>(_rxBuffer.data());
    switch (msgId)
    {
        case RADIO_TO_CTRL_ALIVE_ID:
        {
            RadioToCtrlAliveMessage msgParsed = *reinterpret_cast<RadioToCtrlAliveMessage*>(_rxBuffer.data());
            receivedRadioAlive(msgParsed);
            break;
        }
        case RADIO_TO_CTRL_CFG_ID:
        {
            RadioToCtrlConfigMessage msgParsed = *reinterpret_cast<RadioToCtrlConfigMessage*>(_rxBuffer.data());
            receivedRadioConfig(msgParsed);
            break;
        }
        case RADIO_TO_CTRL_ECHO_ID:
        {
            DroneToRadioResponseMessage echoedBack = *reinterpret_cast<DroneToRadioResponseMessage*>(_rxBuffer.data());
            receivedRadioCmdEcho(echoedBack);
            break;
        }
        default:
            break;

    }
    _downlinkTimer->start();
}

void RadioDriver::receivedRadioAlive(RadioToCtrlAliveMessage msgParsed)
{
    emit radioFirmwareVersion(QString("%1.%2-%3").arg(msgParsed.major_v)
                                              .arg(msgParsed.minor_v)
                                              .arg(msgParsed.stage_v).toUpper());

    if (_state == OFF || _state == INIT)
    {
        _configMsg.config_ok = 0;
        _state = CONFIG_MISMATCH;
        emit radioChangedState(CONFIG_MISMATCH);

        _txTimer->start();
        _downlinkTimer->start();
    }
    else if (_state == CONFIG_MISMATCH)
    {
        if (_txTimer)
        {
            _txTimer->stop();
            _txTimer->start();
        }

        if (_downlinkTimer)
        {
            _downlinkTimer->stop();
            _downlinkTimer->start();
        }
    }

    emit droneAlive(msgParsed.drone_alive == 1);
}

void RadioDriver::receivedRadioConfig(RadioToCtrlConfigMessage msgParsed)
{
    bool rxPipeOk = msgParsed.rx_pipe == _configMsg.rx_pipe;
    bool txPipeOk = msgParsed.tx_pipe == _configMsg.tx_pipe;

    if (!rxPipeOk || !txPipeOk)
    {
#if 0
        printf("config mismatch due to failed config read\n");
        printf("expected rx(0x%lX)\n", _configMsg.rx_pipe);
        printf("actual rx(0x%lX)\n", msgParsed.rx_pipe);
        printf("expected tx(0x%lX)\n", _configMsg.tx_pipe);
        printf("actual tx(0x%lX)\n\n", msgParsed.tx_pipe);
#endif
        _configMsg.config_ok = 0;
        _state = CONFIG_MISMATCH;
        emit radioChangedState(CONFIG_MISMATCH);
    }
    else
    {
        _configMsg.config_ok = 1;
        if (_state != RUNNING)
        {
            _state = TO_RUNNING;
        }
    }
}

void RadioDriver::receivedRadioCmdEcho(DroneToRadioResponseMessage msgParsed)
{
    emit droneResponse(msgParsed);
}

bool RadioDriver::saveChunk(QByteArray chunk)
{
    bool end = false;

    for (int i = 0; i < chunk.size() && !end; i++)
    {
        if (chunk.at(i) == END_MARKER)
        {
            end = true;
        }
        else if (!_gotEnd)
        {
            _rxBuffer.push_back(chunk.at(i));
        }
    }

    return end;
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
