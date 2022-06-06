#include "modules/gui/DroneControllerWindow.h"
#include "modules/settings/settings.h"

#include <QLabel>

DroneControllerWindow::DroneControllerWindow(QWidget *parent) : QMainWindow(parent)
{
    setGeometry(10, 10, Settings::instance()->getAttribute(Settings::Attribute::WINDOW_WIDTH).toInt(), Settings::instance()->getAttribute(Settings::Attribute::WINDOW_HEIGHT).toInt());

    createFrames();

    installEventFilter(this);
}

void DroneControllerWindow::createFrames()
{
    Settings* s = Settings::instance();

    _localFrame = new ControllerFrame(this, "LOCAL",
                                      {
                                          {ControllerMenu::MenuItemKey::JOYSTICK, "JOYSTICK", 0, {"OFF", "ON"}},
                                          {ControllerMenu::MenuItemKey::RADIO,    "RADIO",    0, {"OFF", "RUNNING", "CONFIG. MISMATCH", "INIT", "UNVALID"}},
                                          {ControllerMenu::MenuItemKey::RADIO_FW_VERSION, "R-FW-VER", 0, {"UNKNOWN"}},
                                          {ControllerMenu::MenuItemKey::RADIO_DEVICE, "R-DEVICE", 0, {s->getAttribute(Settings::Attribute::RADIO_DEVICE)}},
                                          {ControllerMenu::MenuItemKey::RADIO_BAUD, "R-BAUD", 0, {s->getAttribute(Settings::Attribute::RADIO_BAUD)}},
                                          {ControllerMenu::MenuItemKey::RADIO_TX_FREQ, "R-FREQ", 0, {s->getAttribute(Settings::Attribute::RADIO_TX_FREQ)}},
                                          {ControllerMenu::MenuItemKey::RADIO_TX_PIPE, "R-TX PIPE", 0, {s->getAttribute(Settings::Attribute::RADIO_TX_PIPE)}},
                                          {ControllerMenu::MenuItemKey::RADIO_RX_PIPE, "R-RX PIPE", 0, {s->getAttribute(Settings::Attribute::RADIO_RX_PIPE)}},
                                      });

    _localFrame->place(0, 0,
        Settings::instance()->getAttribute(Settings::Attribute::WINDOW_WIDTH).toInt() /2,
        Settings::instance()->getAttribute(Settings::Attribute::WINDOW_HEIGHT).toInt());

    _remoteFrame = new ControllerFrame(this, "REMOTE",
                                       {
                                           {ControllerMenu::MenuItemKey::DRONE_STATUS,      "DRONE", 0, {"OFF", "ON"}},
                                           {ControllerMenu::MenuItemKey::DRONE_FW_VERSION,  "FW-VER", 0, {"UNKNOWN"}},
                                           {ControllerMenu::MenuItemKey::DRONE_MOTOR_STATUS, "MOTORS", 0, {"DISARMED", "ARMED"}}
                                       });

    _remoteFrame->place(Settings::instance()->getAttribute(Settings::Attribute::WINDOW_WIDTH).toInt() /2, 0,
        Settings::instance()->getAttribute(Settings::Attribute::WINDOW_WIDTH).toInt() /2,
        Settings::instance()->getAttribute(Settings::Attribute::WINDOW_HEIGHT).toInt());

    _jsFrame = new JoystickFrame(_localFrame);
    _droneFrame = new DroneFrame(_remoteFrame);
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

void DroneControllerWindow::onJoystickMsgOut(CtrlToRadioCommandMessage msgOut)
{
    _jsFrame->updateMessageToDisplay(msgOut);
}

void DroneControllerWindow::onDroneAlive(bool alive)
{
    _remoteFrame->updateMenuItem(ControllerMenu::MenuItemKey::DRONE_STATUS, alive);
}

void DroneControllerWindow::onDroneResponseMessage(DroneToRadioResponseMessage msgIn)
{
    _remoteFrame->updateMenuItem(ControllerMenu::MenuItemKey::DRONE_FW_VERSION, QString("%1.%2-%3")
                                                                                        .arg(msgIn.fw_major_v)
                                                                                        .arg(msgIn.fw_minor_v)
                                                                                        .arg(msgIn.fw_stage_v));

    _remoteFrame->updateMenuItem(ControllerMenu::MenuItemKey::DRONE_MOTOR_STATUS, msgIn.motors_armed);
    _droneFrame->updateMessageToDisplay(msgIn);
}


bool DroneControllerWindow::eventFilter(QObject* target, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
          QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
          onKeyPressed(keyEvent->key());

          return true;
    }

    return QMainWindow::eventFilter(target, event);
}

void DroneControllerWindow::onKeyPressed(int key)
{
    if (QKEY_ESCAPE == key)
    {
        emit guiExit();
    }
}
