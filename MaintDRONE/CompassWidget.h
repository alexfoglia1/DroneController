#ifndef COMPASS_H
#define COMPASS_H

#include <qlabel.h>
#include <qpixmap.h>


class CompassWidget :
    public QLabel
{
    Q_OBJECT
public:

    CompassWidget(QWidget* parent);
    void setGeometry(const QRect& r);
    void setYaw(float yaw);
protected:
    virtual void paintEvent(QPaintEvent* event) override;

private:
    float _yaw;
    QPixmap* _pixmap;
};

#endif //COMPASS_H
