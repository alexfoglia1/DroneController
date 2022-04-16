#ifndef CONTROLLER_MENU_H
#define CONTROLLER_MENU_H

#include <QFrame>
#include <QString>
#include <QVariant>
#include <vector>

class ControllerMenu : public QFrame
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
        DRONE_CONTROLLER,
        DRONE_MOTOR_1_POWER,
        DRONE_MOTOR_2_POWER,
        DRONE_MOTOR_3_POWER,
        DRONE_MOTOR_4_POWER,
        DRONE_MOTOR_1_SPEED,
        DRONE_MOTOR_2_SPEED,
        DRONE_MOTOR_3_SPEED,
        DRONE_MOTOR_4_SPEED,
        DRONE_VOLTAGE_IN,
        DRONE_HEADING,
        DRONE_ROLL,
        DRONE_PITCH,
    };

    typedef struct
    {
        MenuItemKey key;
        QString displayName;
        int currentValueIndex;
        std::vector<QVariant> values;
    } ControllerMenuItem;


    explicit ControllerMenu(QWidget* parent = nullptr, std::vector<ControllerMenuItem> menuItems = std::vector<ControllerMenuItem>());
    int menuItemCount();
    void updateItem(MenuItemKey key, QVariant newValue);
    void paintEvent(QPaintEvent* paintEvent) override;

private:
    std::vector<ControllerMenuItem> _menuItems;
};

#endif //CONTROLLER_MENU_H

