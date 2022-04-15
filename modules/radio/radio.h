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
        NOT_CONFIGURED,
        CONFIG_MISMATCH,
        RUNNING
    };

    RadioDriver();
    bool init();

public slots:
    void receiveData();
    void downlink();
    void transmitData();
    void onJsBtnPressed(int btn);

signals:
    void radioChangedState(int newState);
    void radioFirmwareVersion(QString version);

private:
    QSerialPort* _serialPort;
    QTimer* _downlinkTimer;
    QTimer* _txTimer;
    QString _serialPortName;
    CtrlToRadioConfig _configMsg;
    CtrlToRadioCommand _commandMsg;

    bool _gotStart;
    bool _gotEnd;
    int _baudRate;
    int _txTimeoutMillis;
    int _rxTimeoutMillis;

    QByteArray _rxBuffer;
    QByteArray _txBuffer;

    RadioState _state;

    void receivedRadioAlive(RadioToCtrlAliveMessage msgParsed);
    void receivedRadioAck(RadioToCtrlAckMessage msgParsed);
    void receivedRadioConfig(RadioToCtrlConfig msgParsed);

    void saveChunk(QByteArray chunk);
    void dataIngest();
    void clearTxBuffer();
    void setupTxBuffer(char* data, quint64 size);
};

#endif //RADIO_H

