#ifndef DRONECONTROLLERWINDOW_H
#define DRONECONTROLLERWINDOW_H

#include "modules/gui/widgets/ControllerFrame.h"
#include "modules/gui/widgets/JoystickFrame.h"
#include "modules/radio/proto.h"

#include <QMainWindow>

class DroneControllerWindow : public QMainWindow
{
    Q_OBJECT
public:
    static const int WINDOW_WIDTH = 1200;
    static const int WINDOW_HEIGHT = 650;

    explicit DroneControllerWindow(QWidget *parent = nullptr);
    void closeEvent(QCloseEvent* event) override;

public slots:
    void onRadioFirmwareVersion(QString version);
    void onRadioChangedState(int newState);
    void onJoystickConnected(bool connected);
    void onJoystickMsgOut(CtrlToRadioCommandMessage msgOut);
    void onDroneResponseMessage(DroneToRadioResponseMessage msgIn);

private:
    ControllerFrame* _localFrame;
    ControllerFrame* _remoteFrame;
    JoystickFrame*   _jsFrame;

    void createFrames();

signals:
    void guiExit();
};

#endif // DRONECONTROLLERWINDOW_H
