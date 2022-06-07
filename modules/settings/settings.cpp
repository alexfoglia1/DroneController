#include "modules/settings/settings.h"
#include "modules/settings/rapidxml.hpp"

#include <QFile>
#include <QTextStream>
#include <string.h>

using namespace rapidxml;

const QMap<QString, Settings::Attribute> Settings::_xmlToAttribute =
{
    {"wwidth", WINDOW_WIDTH},
    {"wheight", WINDOW_HEIGHT},
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
    {GUI, "gui"},
    {JOYSTICK, "joystick"},
    {RADIO, "radio"}
};

Settings* Settings::instance()
{
    static Settings* settings = new Settings();

    return settings;
}

Settings::Settings()
{
    _attributes =
    {
        {WINDOW_WIDTH, 1200},
        {WINDOW_HEIGHT, 650},
        {JOYSTICK_DEAD_CENTER_ZONE, 2000},
        {JOYSTICK_BTN_CROSS, 0},
        {JOYSTICK_BTN_CIRCLE, 0},
        {JOYSTICK_BTN_TRIANGLE, 0},
        {JOYSTICK_BTN_SQUARE, 0},
        {JOYSTICK_BTN_L1, 0},
        {JOYSTICK_BTN_R1, 0},
        {JOYSTICK_BTN_L2, 0},
        {JOYSTICK_BTN_R2, 0},
        {JOYSTICK_BTN_L3, 0},
        {JOYSTICK_BTN_R3, 0},
        {JOYSTICK_BTN_SHARE, 0},
        {JOSTICK_BTN_OPT, 0},
        {JOYSTICK_BTN_PS, 0},
        {JOYSTICK_L2_AXIS, 0},
        {JOYSTICK_R2_AXIS, 0},
        {JOYSTICK_L3_X_AXIS, 0},
        {JOYSTICK_L3_Y_AXIS, 0},
        {JOYSTICK_R3_X_AXIS, 0},
        {JOYSTICK_R3_Y_AXIS, 0},
        {RADIO_DEVICE, ""},
        {RADIO_BAUD, 0},
        {RADIO_TX_FREQ, 0},
        {RADIO_TX_PIPE, quint64(0)},
        {RADIO_RX_PIPE, quint64(0)}
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
    char* xmlContent = new char [(int)(ba.size() + 1L)];
    memset(xmlContent, 0x00, (int)(ba.size() + 1L));
    memcpy(xmlContent, ba.data(), ba.size());

    xml_document<> doc;

    try
    {
        doc.parse<0>(xmlContent);
    }  catch (std::exception)
    {
        return false;
    }

    xml_node<>* nodeGui = doc.first_node(_settingsTypeToXml[GUI].toStdString().c_str());
    for (xml_attribute<>* attr = nodeGui->first_attribute();
        attr; attr = attr->next_attribute())
    {
        if (_xmlToAttribute.contains(QString(attr->name())))
        {
            Attribute actual = _xmlToAttribute[attr->name()];
            _attributes[actual] = attr->value();
        }
    }

    xml_node<> *nodeJs = doc.first_node(_settingsTypeToXml[JOYSTICK].toStdString().c_str());
    for (xml_attribute<> *attr = nodeJs->first_attribute();
         attr; attr = attr->next_attribute())
    {
        if (_xmlToAttribute.contains(QString(attr->name())))
        {
            Attribute actual = _xmlToAttribute[attr->name()];
            QVariant actualValue = attr->value();
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


    delete[] xmlContent;
    return true;
}


QVariant Settings::getAttribute(Attribute a)
{
    return _attributes[a];
}
