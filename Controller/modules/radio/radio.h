#ifndef RADIO_H
#define RADIO_H

#include "modules/radio/proto.h"

#include <QSerialPort>
#include <QTimer>

class RadioDriver : public QObject
{
    Q_OBJECT
public:
    enum RadioState
    {
        OFF,
        INIT,
        CONFIG_MISMATCH,
        TO_RUNNING,
        RUNNING
    };

    RadioDriver();
    bool init();

public slots:
    void receiveData();
    void downlink();
    void transmitData();
    void onJsBtnPressed(int btn);
    void onJsMessageUpdate(CtrlToRadioCommandMessage msgOut);

signals:
    void radioChangedState(int newState);
    void radioFirmwareVersion(QString version);
    void droneResponse(DroneToRadioResponseMessage responseMsg);
    void droneAlive(bool alive);

private:
    QSerialPort* _serialPort;
    QTimer* _downlinkTimer;
    QTimer* _txTimer;
    QString _serialPortName;
    CtrlToRadioConfigMessage _configMsg;
    CtrlToRadioCommandMessage _commandMsg;

    bool _gotStart;
    bool _gotEnd;
    int _baudRate;
    int _txTimeoutMillis;
    int _rxTimeoutMillis;

    QByteArray _rxBuffer;
    QByteArray _txBuffer;

    RadioState _state;

    void receivedRadioAlive(RadioToCtrlAliveMessage msgParsed);
    void receivedRadioConfig(RadioToCtrlConfigMessage msgParsed);
    void receivedRadioCmdEcho(DroneToRadioResponseMessage msgParsed);

    bool saveChunk(QByteArray chunk); //<< Returns true if chunk contains endMarker

    void dataIngest();
    void clearTxBuffer();
    void setupTxBuffer(char* data, quint64 size);
};

#endif //RADIO_H

