#include "modules/settings/settings.h"
#include "modules/settings/rapidxml.hpp"

#include <QFile>
#include <QTextStream>
#include <string.h>

using namespace rapidxml;

const QMap<QString, Settings::Attribute> Settings::_xmlToAttribute =
{
    {"deadcenterzone", JOYSTICK_DEAD_CENTER_ZONE},
    {"cross", JOYSTICK_BTN_CROSS},
    {"square", JOYSTICK_BTN_SQUARE},
    {"triangle", JOYSTICK_BTN_TRIANGLE},
    {"circle", JOYSTICK_BTN_CIRCLE},
    {"l1", JOYSTICK_BTN_L1},
    {"r1", JOYSTICK_BTN_R1},
    {"l2", JOYSTICK_BTN_L2},
    {"r2", JOYSTICK_BTN_R2},
    {"l3", JOYSTICK_BTN_L3},
    {"r3", JOYSTICK_BTN_R3},
    {"share", JOYSTICK_BTN_SHARE},
    {"options", JOSTICK_BTN_OPT},
    {"ps", JOYSTICK_BTN_PS},
    {"l2axis", JOYSTICK_L2_AXIS},
    {"r2axis", JOYSTICK_R2_AXIS},
    {"l3x", JOYSTICK_L3_X_AXIS},
    {"l3y", JOYSTICK_L3_Y_AXIS},
    {"r3x", JOYSTICK_R3_X_AXIS},
    {"r3y", JOYSTICK_R3_Y_AXIS},
    {"serialport", RADIO_DEVICE},
    {"baud", RADIO_BAUD},
    {"freq", RADIO_TX_FREQ},
    {"txpipe", RADIO_TX_PIPE},
    {"rxpipe", RADIO_RX_PIPE}
};


const QMap<Settings::SettingsType, QString> Settings::_settingsTypeToXml =
{
    {JOYSTICK, "joystick"},
    {RADIO, "radio"}
};

Settings Settings::instance()
{
    static Settings settings;

    return settings;
}

Settings::Settings()
{
    _attributes =
    {
        {JOYSTICK_DEAD_CENTER_ZONE, 2000},
        {JOYSTICK_BTN_CROSS, 0},
        {JOYSTICK_BTN_CIRCLE, 1},
        {JOYSTICK_BTN_TRIANGLE, 2},
        {JOYSTICK_BTN_SQUARE, 3},
        {JOYSTICK_BTN_L1, 4},
        {JOYSTICK_BTN_R1, 5},
        {JOYSTICK_BTN_L2, 6},
        {JOYSTICK_BTN_R2, 7},
        {JOYSTICK_BTN_L3, 8},
        {JOYSTICK_BTN_R3, 9},
        {JOYSTICK_BTN_SHARE, 10},
        {JOSTICK_BTN_OPT, 11},
        {JOYSTICK_BTN_PS, 12},
        {JOYSTICK_L2_AXIS, 2},
        {JOYSTICK_R2_AXIS, 5},
        {JOYSTICK_L3_X_AXIS, 0},
        {JOYSTICK_L3_Y_AXIS, 1},
        {JOYSTICK_R3_X_AXIS, 3},
        {JOYSTICK_R3_Y_AXIS, 4},
        {RADIO_DEVICE, "/dev/ttyUSB0"},
        {RADIO_BAUD, 9600},
        {RADIO_TX_FREQ, 50},
        {RADIO_TX_PIPE, quint64(0xE6E6E6E6E6E6)},
        {RADIO_RX_PIPE, quint64(0x6E6E6E6E6E6E)}
    };
}

bool Settings::readSettings(const char *filename)
{
    QFile f(filename);
    if (!f.exists())
    {
        return false;
    }

    f.open(QIODevice::OpenModeFlag::ReadOnly);
    QTextStream in(&f);
    QString xml = in.readAll();
    f.close();

    /** QString to C String for the xml library ... **/
    QByteArray ba = xml.toLocal8Bit();
    char xmlContent[ba.size() + 1];
    memset(xmlContent, 0x00, ba.size() + 1);
    memcpy(xmlContent, ba.data(), ba.size());

    xml_document<> doc;

    try
    {
        doc.parse<0>(xmlContent);
    }  catch (std::exception)
    {
        return false;
    }


    xml_node<> *nodeJs = doc.first_node(_settingsTypeToXml[JOYSTICK].toStdString().c_str());
    for (xml_attribute<> *attr = nodeJs->first_attribute();
         attr; attr = attr->next_attribute())
    {
        if (_xmlToAttribute.contains(QString(attr->name())))
        {
            Attribute actual = _xmlToAttribute[attr->name()];
            _attributes[actual] = attr->value();
        }
    }

    xml_node<> *nodeRadio = doc.first_node(_settingsTypeToXml[RADIO].toStdString().c_str());
    for (xml_attribute<> *attr = nodeRadio->first_attribute();
         attr; attr = attr->next_attribute())
    {
        if (_xmlToAttribute.contains(attr->name()))
        {
            Attribute actual = _xmlToAttribute[attr->name()];
            _attributes[actual] = attr->value();
        }
    }

    return true;
}


QVariant Settings::getAttribute(Attribute a)
{
    return _attributes[a];
}
