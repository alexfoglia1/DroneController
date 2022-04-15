#include "modules/gui/widgets/ControllerMenu.h"
#include "modules/radio/radio.h"

#include <QPainter>

ControllerMenu::ControllerMenu(QWidget* parent, std::vector<ControllerMenuItem> menuItems) :
    QFrame(parent)
{
    for(auto& menuItem : menuItems)
    {
        _menuItems.push_back(menuItem);
    }
}

int ControllerMenu::menuItemCount()
{
    return _menuItems.size();
}

void ControllerMenu::updateItem(MenuItemKey key, QVariant newValue)
{
    int index = -1;

    for (int i = 0; i < menuItemCount(); i++)
    {
        if (key == _menuItems[i].key)
        {
            index = i;
        }
    }

    if (-1 == index)
    {
        return;
    }

    ControllerMenuItem* item = &_menuItems[index];
    int oldValueIndex = item->currentValueIndex;

    switch(key)
    {
        /** ON/OFF Menu Item to be updated **/
        case MenuItemKey::JOYSTICK:
        item->currentValueIndex = newValue.toBool() ? 1 : 0;
        break;
        /** MULTI STATE Menu Item to be updated **/
        case MenuItemKey::RADIO:
        item->currentValueIndex = newValue.toInt() == RadioDriver::RadioState::OFF ? 0 :
                                  newValue.toInt() == RadioDriver::RadioState::NOT_CONFIGURED ? 1:
                                  newValue.toInt() == RadioDriver::RadioState::RUNNING ? 2 :
                                  newValue.toInt() == RadioDriver::RadioState::CONFIG_MISMATCH ? 3 : 4;
        break;
        /** CUSTOM STRING Menu Item to be updated **/
        case MenuItemKey::RADIO_FW_VERSION:
        case MenuItemKey::RADIO_BAUD:
        case MenuItemKey::RADIO_DEVICE:
        case MenuItemKey::RADIO_RX_PIPE:
        case MenuItemKey::RADIO_TX_PIPE:
        case MenuItemKey::RADIO_TX_FREQ:
        item->values.clear();
        item->values.push_back(newValue);
        break;
    default:
        break;
    }

    if (oldValueIndex != item->currentValueIndex)
    {
        repaint();
    }
}

void ControllerMenu::paintEvent(QPaintEvent* paintEvent)
{
    QFrame::paintEvent(paintEvent);

    QPainter painter(this);
    int y = 25;
    for(auto& menuItem : _menuItems)
    {
        painter.drawText(10, y, menuItem.displayName);
        QVariant itemValue = menuItem.values[menuItem.currentValueIndex];

        switch (menuItem.key)
        {
            /** Display On/Off Menu Items **/
            case MenuItemKey::JOYSTICK:
                if (itemValue.toBool())
                {
                    painter.drawText(100, y, QString("ON"));
                }
                else
                {
                    painter.drawText(100, y, QString("OFF"));
                }

            break;

            /** Display custom string menu item **/
            case MenuItemKey::RADIO_FW_VERSION:
            case MenuItemKey::RADIO_DEVICE:
            case MenuItemKey::RADIO_BAUD:
            case MenuItemKey::RADIO_TX_FREQ:
            case MenuItemKey::RADIO:
                painter.drawText(100, y, itemValue.toString());
            break;

            /** Display Hex menu items **/
            case MenuItemKey::RADIO_TX_PIPE:
            case MenuItemKey::RADIO_RX_PIPE:
                painter.drawText(100, y,
                                 QString("0x%1").arg(QString::number(itemValue.toULongLong(), 16).toUpper()));
                break;
            default:
                break;
        }

        y += 25;
    }
}
