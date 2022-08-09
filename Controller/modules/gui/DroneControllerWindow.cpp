#include "modules/gui/DroneControllerWindow.h"
#include "modules/settings/settings.h"
#include "modules/radio/radio.h"

#include <QLabel>

DroneControllerWindow::DroneControllerWindow(QWidget *parent) : QMainWindow(parent)
{
    setGeometry(10, 10, 800, 600);

    createFrames();

    installEventFilter(this);
}

void DroneControllerWindow::applySettings(Settings *settings)
{
    setGeometry(10, 10, settings->getAttribute(Settings::Attribute::WINDOW_WIDTH).toInt(), settings->getAttribute(Settings::Attribute::WINDOW_HEIGHT).toInt());

    _localFrame->updateMenuItem(Menu::MenuItemKey::RADIO_DEVICE, settings->getAttribute(Settings::Attribute::RADIO_DEVICE));
    _localFrame->updateMenuItem(Menu::MenuItemKey::RADIO_BAUD, settings->getAttribute(Settings::Attribute::RADIO_BAUD));
    _localFrame->updateMenuItem(Menu::MenuItemKey::RADIO_TX_FREQ, settings->getAttribute(Settings::Attribute::RADIO_TX_FREQ));
    _localFrame->updateMenuItem(Menu::MenuItemKey::RADIO_TX_PIPE, QString("0x%1").arg(QString::number(settings->getAttribute(Settings::Attribute::RADIO_TX_PIPE).toULongLong(), 16).toUpper()));
    _localFrame->updateMenuItem(Menu::MenuItemKey::RADIO_RX_PIPE, QString("0x%1").arg(QString::number(settings->getAttribute(Settings::Attribute::RADIO_RX_PIPE).toULongLong(), 16).toUpper()));
}

void DroneControllerWindow::createFrames()
{
    _localFrame = new Frame(this, "LOCAL",
                                      {
                                          {Menu::MenuItemKey::JOYSTICK, "JOYSTICK", "OFF"},
                                          {Menu::MenuItemKey::RADIO,    "RADIO",    "OFF"},
                                          {Menu::MenuItemKey::RADIO_FW_VERSION, "R-FW-VER", "UNKNOWN"},
                                          {Menu::MenuItemKey::RADIO_DEVICE, "R-DEVICE", "UNKNOWN"},
                                          {Menu::MenuItemKey::RADIO_BAUD, "R-BAUD", "UNKNOWN"},
                                          {Menu::MenuItemKey::RADIO_TX_FREQ, "R-FREQ", "UNKNOWN"},
                                          {Menu::MenuItemKey::RADIO_TX_PIPE, "R-TX PIPE", "UNKNOWN"},
                                          {Menu::MenuItemKey::RADIO_RX_PIPE, "R-RX PIPE", "UNKNOWN"},
                                      });

    _localFrame->place(0, 0,
        Settings::instance()->getAttribute(Settings::Attribute::WINDOW_WIDTH).toInt() /2,
        Settings::instance()->getAttribute(Settings::Attribute::WINDOW_HEIGHT).toInt());

    _remoteFrame = new Frame(this, "REMOTE",
                                       {
                                           {Menu::MenuItemKey::DRONE_STATUS,      "DRONE", "OFF"},
                                           {Menu::MenuItemKey::DRONE_FW_VERSION,  "FW-VER", "UNKNOWN"},
                                           {Menu::MenuItemKey::DRONE_MOTOR_STATUS, "MOTORS", "UNKNOWN"}
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
    QString state("OFF");
    switch (RadioDriver::RadioState(newState))
    {
        case RadioDriver::RadioState::OFF: state = QString("OFF"); break;
        case RadioDriver::RadioState::INIT: state = QString("INIT"); break;
        case RadioDriver::RadioState::CONFIG_MISMATCH: state = QString("CONFIG MISMATCH"); break;
        case RadioDriver::RadioState::RUNNING: state = QString("RUNNING"); break;
        default: state = QString("OFF"); break;
    }

    _localFrame->updateMenuItem(Menu::MenuItemKey::RADIO, state);
}

void DroneControllerWindow::onJoystickConnected(bool connected)
{
    _localFrame->updateMenuItem(Menu::MenuItemKey::JOYSTICK, connected ? "ON" : "OFF");
}

void DroneControllerWindow::onJoystickMsgOut(CtrlToRadioCommandMessage msgOut)
{
    _jsFrame->updateMessageToDisplay(msgOut);
}

void DroneControllerWindow::onDroneAlive(bool alive)
{
    _remoteFrame->updateMenuItem(Menu::MenuItemKey::DRONE_STATUS, alive ? "ON" : "OFF");
}

void DroneControllerWindow::onDroneResponseMessage(DroneToRadioResponseMessage msgIn)
{
    _remoteFrame->updateMenuItem(Menu::MenuItemKey::DRONE_FW_VERSION, QString("%1.%2-%3")
                                                                                        .arg(msgIn.fw_major_v)
                                                                                        .arg(msgIn.fw_minor_v)
                                                                                        .arg(msgIn.fw_stage_v));

    _remoteFrame->updateMenuItem(Menu::MenuItemKey::DRONE_MOTOR_STATUS, msgIn.motors_armed ? "ARMED" : "DISARMED");
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
