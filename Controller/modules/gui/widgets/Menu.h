#ifndef CONTROLLER_MENU_H
#define CONTROLLER_MENU_H

#include <QFrame>
#include <QString>
#include <QVariant>
#include <vector>

class Menu : public QFrame
{
    Q_OBJECT

public:
    enum MenuItemKey
    {
        /** Local **/
        JOYSTICK = 0,
        RADIO,
        RADIO_FW_VERSION,

        /** Settings **/
        RADIO_DEVICE,
        RADIO_BAUD,
        RADIO_TX_FREQ,
        RADIO_TX_PIPE,
        RADIO_RX_PIPE,

        /** Remote **/
        DRONE_STATUS,
        DRONE_MOTOR_STATUS,
        DRONE_FW_VERSION,
        DRONE_MOTOR_1_POWER,
        DRONE_MOTOR_2_POWER,
        DRONE_MOTOR_3_POWER,
        DRONE_MOTOR_4_POWER,
        DRONE_MOTOR_1_SPEED,
        DRONE_MOTOR_2_SPEED,
        DRONE_MOTOR_3_SPEED,
        DRONE_MOTOR_4_SPEED,
        DRONE_HEADING,
        DRONE_ROLL,
        DRONE_PITCH,
    };

    typedef struct
    {
        MenuItemKey key;
        QVariant displayName;
        QVariant value;
    } MenuItem;


    explicit Menu(QWidget* parent = nullptr, std::vector<MenuItem> menuItems = std::vector<MenuItem>());
    int  menuItemCount();
    void updateItem(MenuItemKey key, QVariant newValue);
    void paintEvent(QPaintEvent* paintEvent) override;

private:
    std::vector<MenuItem> _menuItems;
};

#endif //CONTROLLER_MENU_H

