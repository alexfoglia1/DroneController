#include "modules/gui/DroneControllerWindow.h"
#include "modules/settings/settings.h"

#include <QLabel>

DroneControllerWindow::DroneControllerWindow(QWidget *parent) : QMainWindow(parent)
{
    setGeometry(10, 10, DroneControllerWindow::WINDOW_WIDTH, DroneControllerWindow::WINDOW_HEIGHT);

    createFrames();
}

void DroneControllerWindow::createFrames()
{
    Settings* s = Settings::instance();

    _localFrame = new ControllerFrame(this, "LOCAL",
                                      {
                                          {ControllerMenu::MenuItemKey::JOYSTICK, "JOYSTICK", 0, {false, true}},
                                          {ControllerMenu::MenuItemKey::RADIO,    "RADIO",    0, {"OFF", "NOT CONFIGURED", "RUNNING", "CONFIG. MISMATCH", "UNVALID"}},
                                          {ControllerMenu::MenuItemKey::RADIO_FW_VERSION, "R-FW-VER", 0, {"UNKNOWN"}},
                                          {ControllerMenu::MenuItemKey::RADIO_DEVICE, "R-DEVICE", 0, {s->getAttribute(Settings::Attribute::RADIO_DEVICE)}},
                                          {ControllerMenu::MenuItemKey::RADIO_BAUD, "R-BAUD", 0, {s->getAttribute(Settings::Attribute::RADIO_BAUD)}},
                                          {ControllerMenu::MenuItemKey::RADIO_TX_FREQ, "R-FREQ", 0, {s->getAttribute(Settings::Attribute::RADIO_TX_FREQ)}},
                                          {ControllerMenu::MenuItemKey::RADIO_TX_PIPE, "R-TX PIPE", 0, {s->getAttribute(Settings::Attribute::RADIO_TX_PIPE)}},
                                          {ControllerMenu::MenuItemKey::RADIO_RX_PIPE, "R-RX PIPE", 0, {s->getAttribute(Settings::Attribute::RADIO_RX_PIPE)}},
                                      });

    _localFrame->place(0, 0,
                       DroneControllerWindow::WINDOW_WIDTH/2, DroneControllerWindow::WINDOW_HEIGHT);

    _remoteFrame = new ControllerFrame(this, "REMOTE");
    _remoteFrame->place(DroneControllerWindow::WINDOW_WIDTH/2, 0,
                        DroneControllerWindow::WINDOW_WIDTH/2, DroneControllerWindow::WINDOW_HEIGHT);

    _jsFrame = new JoystickFrame(_localFrame);
}

void DroneControllerWindow::onRadioFirmwareVersion(QString version)
{
    _localFrame->updateMenuItem(ControllerMenu::MenuItemKey::RADIO_FW_VERSION, version);
}

void DroneControllerWindow::onRadioChangedState(int newState)
{
    _localFrame->updateMenuItem(ControllerMenu::MenuItemKey::RADIO, newState);
}

void DroneControllerWindow::onJoystickConnected(bool connected)
{
    _localFrame->updateMenuItem(ControllerMenu::MenuItemKey::JOYSTICK, connected);
}

void DroneControllerWindow::onJoystickMsgOut(CtrlToRadioCommand msgOut)
{
    _jsFrame->updateMessageToDisplay(msgOut);
}
