#include "modules/radio/proto.h"
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
    qRegisterMetaType<CtrlToRadioCommand>();

    /** Read settings **/
    Settings settings = Settings::instance();
    QFile settingsFile("Settings.xml");
    if (!settingsFile.exists())
    {
        std::cout << "Settings file does not exists" << std::endl;
    }
    else if (!settings.readSettings("Settings.xml"))
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
    QObject::connect(&js, SIGNAL(msgOut(CtrlToRadioCommand)), &window, SLOT(onJoystickMsgOut(CtrlToRadioCommand)));

    /** Launch joystick control **/
    js.start();

    /** Connect app exit to joystick exit **/
    QObject::connect(&app, &QApplication::aboutToQuit, &js, [&js](){js.updateState(Joystick::js_thread_state_t::EXIT);});

    /** Launch app **/
    return app.exec();
}