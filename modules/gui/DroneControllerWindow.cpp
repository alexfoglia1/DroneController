#include "DroneControllerWindow.h"

#include <QLabel>

DroneControllerWindow::DroneControllerWindow(QWidget *parent) : QMainWindow(parent)
{
    setGeometry(10, 10, DroneControllerWindow::WINDOW_WIDTH, DroneControllerWindow::WINDOW_HEIGHT);

    createFrames();
}

void DroneControllerWindow::createFrames()
{
    _localFrame = new ControllerFrame(this, "LOCAL",
                                      {
                                          {ControllerMenu::MenuItemKey::JOYSTICK, "JOYSTICK", 0, {false, true}},
                                          {ControllerMenu::MenuItemKey::RADIO,    "RADIO",    0, {false, true}},
                                      });
    _localFrame->place(0, 0,
                       DroneControllerWindow::WINDOW_WIDTH/2, DroneControllerWindow::WINDOW_HEIGHT);

    _remoteFrame = new ControllerFrame(this, "REMOTE");
    _remoteFrame->place(DroneControllerWindow::WINDOW_WIDTH/2, 0,
                        DroneControllerWindow::WINDOW_WIDTH/2, DroneControllerWindow::WINDOW_HEIGHT);

    _jsFrame = new JoystickFrame(_localFrame);
}

void DroneControllerWindow::onJoystickConnected(bool connected)
{
    _localFrame->updateMenuItem(ControllerMenu::MenuItemKey::JOYSTICK, connected);
}

void DroneControllerWindow::onJoystickMsgOut(CtrlToRadioMsg msgOut)
{
    _jsFrame->updateMessageToDisplay(msgOut);
}
