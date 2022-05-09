#ifndef DRONECONTROLLERWINDOW_H
#define DRONECONTROLLERWINDOW_H

#include "modules/gui/widgets/ControllerFrame.h"
#include "modules/gui/widgets/JoystickFrame.h"
#include "modules/gui/widgets/DroneFrame.h"
#include "modules/radio/proto.h"

#include <QMainWindow>

class DroneControllerWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit DroneControllerWindow(QWidget *parent = nullptr);
    void closeEvent(QCloseEvent* event) override;

public slots:
    void onRadioFirmwareVersion(QString version);
    void onRadioChangedState(int newState);
    void onJoystickConnected(bool connected);
    void onJoystickMsgOut(CtrlToRadioCommandMessage msgOut);
    void onDroneAlive(bool alive);
    void onDroneResponseMessage(DroneToRadioResponseMessage msgIn);

private:
    ControllerFrame* _localFrame;
    ControllerFrame* _remoteFrame;
    JoystickFrame*   _jsFrame;
    DroneFrame*      _droneFrame;

    void createFrames();

signals:
    void guiExit();
};

#endif // DRONECONTROLLERWINDOW_H
