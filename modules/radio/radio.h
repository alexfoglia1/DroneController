#ifndef RADIO_H
#define RADIO_H

#include <QSerialPort>
#include <QTimer>

class RadioDriver : public QObject
{
    Q_OBJECT
public:
    RadioDriver();
    bool init();

public slots:
    void receiveData();
    void handleError();
    void downlink();

private:
    QSerialPort* _serialPort;
    QTimer* _rxTimer;
    QString _serialPortName;
    int _baudRate;
};

#endif //RADIO_H

