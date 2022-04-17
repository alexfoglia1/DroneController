#include "modules/radio/proto.h"
#include "modules/radio/radio.h"
#include "modules/app/app.h"
#include "modules/gui/DroneControllerWindow.h"
#include "modules/joystick/joystick.h"
#include "modules/settings/settings.h"

#include <iostream>
#include <QFile>
#include <QApplication>

int main(int argc, char** argv)
{

    QApplication app(argc, argv);
    app.setApplicationName(PROJECT_NAME);
    app.setApplicationVersion(QString("%1.%2-%3").arg(MAJOR_VERSION).arg(MINOR_VERSION).arg(STAGE_VERSION));
    app.setApplicationDisplayName(QString("%1 %2").arg(PROJECT_NAME).arg(app.applicationVersion()));

    /** Register metatypes **/
    qRegisterMetaType<CtrlToRadioCommandMessage>();
    qRegisterMetaType<DroneToRadioResponseMessage>();

    /** Read settings **/
    Settings* settings = Settings::instance();
    QFile settingsFile("Settings.xml");
    if (!settingsFile.exists())
    {
        std::cout << "Settings file does not exists" << std::endl;
    }
    else if (!settings->readSettings("Settings.xml"))
    {
        std::cout << "Cannot parse settings " << std::endl;
    }

    /** Create GUI control **/
    DroneControllerWindow window;
    window.setStyleSheet("background-color: black");
    window.show();

    /** Create joystick control **/
    Joystick js;

    /** Connect joystick to GUI **/
    QObject::connect(&js, SIGNAL(jsConnected(bool)), &window, SLOT(onJoystickConnected(bool)));
    QObject::connect(&js, SIGNAL(msgOut(CtrlToRadioCommandMessage)), &window, SLOT(onJoystickMsgOut(CtrlToRadioCommandMessage)));

    /** Launch joystick control **/
    js.start();

    /** Create radio control **/
    RadioDriver radio;

    /** Connect radio to GUI **/
    QObject::connect(&radio, SIGNAL(radioFirmwareVersion(QString)), &window, SLOT(onRadioFirmwareVersion(QString)));
    QObject::connect(&radio, SIGNAL(radioChangedState(int)), &window, SLOT(onRadioChangedState(int)));

    /** Connect Joystick to radio**/
    QObject::connect(&js, SIGNAL(btnPressed(int)), &radio, SLOT(onJsBtnPressed(int)));
    QObject::connect(&js, SIGNAL(msgOut(CtrlToRadioCommandMessage)), &radio, SLOT(onJsMessageUpdate(CtrlToRadioCommandMessage)));

    QObject::connect(&window, SIGNAL(guiExit()), &js, SLOT(onApplicationQuit()));
    /** Launch app **/
    return app.exec();
}
