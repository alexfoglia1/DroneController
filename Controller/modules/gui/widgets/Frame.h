#ifndef CONTROLLER_FRAME_H
#define CONTROLLER_FRAME_H

#include "modules/gui/widgets/Menu.h"
#include "modules/radio/proto.h"

#include <QFrame>
#include <QLabel>

class Frame : public QFrame
{
    Q_OBJECT
public:
    explicit Frame(QWidget* parent = nullptr, const QString& title = "", std::vector<Menu::MenuItem> menuItems = std::vector<Menu::MenuItem>());
    void place(int x, int y, int w, int h);
    void updateMenuItem(Menu::MenuItemKey key, QVariant newValue);

private:
    QLabel* _lblTitle;
    Menu* _menuSubFrame;

    int _x;
    int _y;
    int _w;
    int _h;

    int labelWidth(int txtLength);
    int labelHeight(int fontSize);
};


#endif
