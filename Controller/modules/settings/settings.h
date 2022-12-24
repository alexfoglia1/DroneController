#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>
#include <QMap>
#include <QVariant>

class Settings
{
public:

    enum Attribute
    {
        /** Gui settings **/
        WINDOW_WIDTH,
        WINDOW_HEIGHT,

        /** Joystick settings **/
        JOYSTICK_DEAD_CENTER_ZONE,
        JOYSTICK_BTN_CROSS,
        JOYSTICK_BTN_SQUARE,
        JOYSTICK_BTN_TRIANGLE,
        JOYSTICK_BTN_CIRCLE,
        JOYSTICK_BTN_L1,
        JOYSTICK_BTN_R1,
        JOYSTICK_BTN_L2,
        JOYSTICK_BTN_R2,
        JOYSTICK_BTN_L3,
        JOYSTICK_BTN_R3,
        JOYSTICK_BTN_SHARE,
        JOSTICK_BTN_OPT,
        JOYSTICK_BTN_PS,
        JOYSTICK_L2_AXIS,
        JOYSTICK_R2_AXIS,
        JOYSTICK_L3_X_AXIS,
        JOYSTICK_L3_Y_AXIS,
        JOYSTICK_R3_X_AXIS,
        JOYSTICK_R3_Y_AXIS,

        /** Radio settings **/
        RADIO_DEVICE,
        RADIO_BAUD,
        RADIO_TX_FREQ,
        RADIO_TX_PIPE,
        RADIO_RX_PIPE
    };

    enum SettingsType
    {
        GUI,
        JOYSTICK,
        RADIO
    };

    static Settings* instance();

    bool readSettings(const char* filename);
    QVariant getAttribute(Attribute a);

private:
    static const QMap<QString, Attribute> _xmlToAttribute;
    static const QMap<SettingsType, QString> _settingsTypeToXml;
    QMap<Attribute, QVariant> _attributes;

    Settings();

};

#endif //SETTINGS_H
