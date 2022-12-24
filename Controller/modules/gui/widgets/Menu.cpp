#include "modules/gui/widgets/Menu.h"
#include "modules/radio/radio.h"

#include <QPainter>

Menu::Menu(QWidget* parent, std::vector<MenuItem> menuItems) :
    QFrame(parent)
{
    for(auto& menuItem : menuItems)
    {
        _menuItems.push_back(menuItem);
    }
}

int Menu::menuItemCount()
{
    return _menuItems.size();
}

void Menu::updateItem(MenuItemKey key, QVariant newValue)
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

    MenuItem* item = &_menuItems[index];
    QVariant oldValue = item->value;
    item->value = newValue;
    if (newValue != oldValue)
    {
        repaint();
    }
}

void Menu::paintEvent(QPaintEvent* paintEvent)
{
    QFrame::paintEvent(paintEvent);

    QPainter painter(this);
    int y = 25;
    for(auto& menuItem : _menuItems)
    {
        painter.drawText(10, y, menuItem.displayName.toString());
        QVariant itemValue = menuItem.value;
        painter.drawText(100, y, itemValue.toString());

        y += 25;
    }
}
