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

    _localFrame = new Frame(this, "LOCAL",
                                      {
                                          {Menu::MenuItemKey::JOYSTICK, "JOYSTICK", 0, {"OFF", "ON"}},
                                          {Menu::MenuItemKey::RADIO,    "RADIO",    0, {"OFF", "RUNNING", "CONFIG. MISMATCH", "INIT", "UNVALID"}},
                                          {Menu::MenuItemKey::RADIO_FW_VERSION, "R-FW-VER", 0, {"UNKNOWN"}},
                                          {Menu::MenuItemKey::RADIO_DEVICE, "R-DEVICE", 0, {s->getAttribute(Settings::Attribute::RADIO_DEVICE)}},
                                          {Menu::MenuItemKey::RADIO_BAUD, "R-BAUD", 0, {s->getAttribute(Settings::Attribute::RADIO_BAUD)}},
                                          {Menu::MenuItemKey::RADIO_TX_FREQ, "R-FREQ", 0, {s->getAttribute(Settings::Attribute::RADIO_TX_FREQ)}},
                                          {Menu::MenuItemKey::RADIO_TX_PIPE, "R-TX PIPE", 0, {s->getAttribute(Settings::Attribute::RADIO_TX_PIPE)}},
                                          {Menu::MenuItemKey::RADIO_RX_PIPE, "R-RX PIPE", 0, {s->getAttribute(Settings::Attribute::RADIO_RX_PIPE)}},
                                      });

    _localFrame->place(0, 0,
        Settings::instance()->getAttribute(Settings::Attribute::WINDOW_WIDTH).toInt() /2,
        Settings::instance()->getAttribute(Settings::Attribute::WINDOW_HEIGHT).toInt());

    _remoteFrame = new Frame(this, "REMOTE",
                                       {
                                           {Menu::MenuItemKey::DRONE_STATUS,      "DRONE", 0, {"OFF", "ON"}},
                                           {Menu::MenuItemKey::DRONE_FW_VERSION,  "FW-VER", 0, {"UNKNOWN"}},
                                           {Menu::MenuItemKey::DRONE_MOTOR_STATUS, "MOTORS", 0, {"DISARMED", "ARMED"}}
                                       });

    _remoteFrame->place(Settings::instance()->getAttribute(Settings::Attribute::WINDOW_WIDTH).toInt() /2, 0,
        Settings::instance()->getAttribute(Settings::Attribute::WINDOW_WIDTH).toInt() /2,
        Settings::instance()->getAttribute(Settings::Attribute::WINDOW_HEIGHT).toInt());

    _jsFrame = new JoystickFrame(_localFrame);
    _droneFrame = new DroneFrame(_remoteFrame);
}

void DroneControllerWindow::onRadioFirmwareVersion(QString version)
{
    _localFrame->updateMenuItem(Menu::MenuItemKey::RADIO_FW_VERSION, version);
}

void DroneControllerWindow::onRadioChangedState(int newState)
{
    _localFrame->updateMenuItem(Menu::MenuItemKey::RADIO, newState);
}

void DroneControllerWindow::onJoystickConnected(bool connected)
{
    _localFrame->updateMenuItem(Menu::MenuItemKey::JOYSTICK, connected);
}

void DroneControllerWindow::onJoystickMsgOut(CtrlToRadioCommandMessage msgOut)
{
    _jsFrame->updateMessageToDisplay(msgOut);
}

void DroneControllerWindow::onDroneAlive(bool alive)
{
    _remoteFrame->updateMenuItem(Menu::MenuItemKey::DRONE_STATUS, alive);
}

void DroneControllerWindow::onDroneResponseMessage(DroneToRadioResponseMessage msgIn)
{
    _remoteFrame->updateMenuItem(Menu::MenuItemKey::DRONE_FW_VERSION, QString("%1.%2-%3")
                                                                                        .arg(msgIn.fw_major_v)
                                                                                        .arg(msgIn.fw_minor_v)
                                                                                        .arg(msgIn.fw_stage_v));

    _remoteFrame->updateMenuItem(Menu::MenuItemKey::DRONE_MOTOR_STATUS, msgIn.motors_armed);
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
