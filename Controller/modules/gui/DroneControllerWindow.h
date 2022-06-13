#ifndef DRONECONTROLLERWINDOW_H
#define DRONECONTROLLERWINDOW_H

#include "modules/gui/widgets/Frame.h"
#include "modules/gui/widgets/JoystickFrame.h"
#include "modules/gui/widgets/DroneFrame.h"
#include "modules/radio/proto.h"

#include <QMainWindow>
#include <QKeyEvent>

#define QKEY_ESCAPE 16777216

class DroneControllerWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit DroneControllerWindow(QWidget *parent = nullptr);
    bool eventFilter(QObject* target, QEvent* event);

public slots:
    void onRadioFirmwareVersion(QString version);
    void onRadioChangedState(int newState);
    void onJoystickConnected(bool connected);
    void onJoystickMsgOut(CtrlToRadioCommandMessage msgOut);
    void onDroneAlive(bool alive);
    void onDroneResponseMessage(DroneToRadioResponseMessage msgIn);

private:
    Frame* _localFrame;
    Frame* _remoteFrame;
    JoystickFrame*   _jsFrame;
    DroneFrame*      _droneFrame;

    void createFrames();
    void onKeyPressed(int key);

signals:
    void guiExit();
};

#endif // DRONECONTROLLERWINDOW_H
