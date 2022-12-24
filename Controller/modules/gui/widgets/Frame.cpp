#include "Frame.h"

Frame::Frame(QWidget* parent, const QString& title, std::vector<Menu::MenuItem> menuItems) : QFrame(parent)
{
    _x = 0;
    _y = 0;
    _w = 0;
    _h = 0;

    setStyleSheet("background-color: gray;"
                  "border-style: outset;"
                  "border-radius: 15px;"
                  "border-width: 2px;"
                  "border-color: black;");

    _lblTitle = new QLabel(this);
    _lblTitle->setText(title);
    _lblTitle->setFont(QFont("Sans Serif", 14, -1, false));

    /** Set title y, width, height: x must be updated in place() **/
    _lblTitle->setGeometry(0, 5, labelWidth(title.length()),
                                 labelHeight(_lblTitle->font().pointSize()));

    _lblTitle->setStyleSheet("background-color: blue;"
                             "color: white;");

    _menuSubFrame = new Menu(this, menuItems);

    /** Set menu x, y, height: width must be updated in place() **/
    _menuSubFrame->setGeometry(5, 2 * _lblTitle->height(), _w, 30 * _menuSubFrame->menuItemCount());

    _menuSubFrame->setStyleSheet("background-color: blue;"
                                 "color: white;");
}

void Frame::place(int x, int y, int w, int h)
{
    _x = x;
    _y = y;
    _w = w;
    _h = h;

    /** Update frame geometry **/
    setGeometry(_x, _y, _w, _h);

    /** Update label x pos **/
    _lblTitle->setGeometry((_w - _lblTitle->width())/2, _lblTitle->y(), _lblTitle->width(), _lblTitle->height());

    /** Update menu width **/
    _menuSubFrame->setGeometry(_menuSubFrame->x(), _menuSubFrame->y(), _w/2, _menuSubFrame->height());
    show();
}

void Frame::updateMenuItem(Menu::MenuItemKey key, QVariant newValue)
{
    _menuSubFrame->updateItem(key, newValue);
}


int Frame::labelWidth(int txtLength)
{
    return 16 * txtLength;
}

int Frame::labelHeight(int fontSize)
{
    return 2 * fontSize;
}
