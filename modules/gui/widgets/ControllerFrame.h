#ifndef CONTROLLER_FRAME_H
#define CONTROLLER_FRAME_H

#include "modules/gui/widgets/ControllerMenu.h"
#include "modules/radio/proto.h"

#include <QFrame>
#include <QLabel>

class ControllerFrame : public QFrame
{
    Q_OBJECT
public:
    explicit ControllerFrame(QWidget* parent = nullptr, const QString& title = "", std::vector<ControllerMenu::ControllerMenuItem> menuItems = std::vector<ControllerMenu::ControllerMenuItem>());
    void place(int x, int y, int w, int h);
    void updateMenuItem(ControllerMenu::MenuItemKey key, bool connected);

private:
    QLabel* _lblTitle;
    ControllerMenu* _menuSubFrame;

    int _x;
    int _y;
    int _w;
    int _h;

    int labelWidth(int txtLength);
    int labelHeight(int fontSize);
};


#endif
