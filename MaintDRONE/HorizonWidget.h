#ifndef HORIZON_H
#define HORIZON_H

#include <qlabel.h>
#include <qpixmap.h>


class HorizonWidget :
    public QLabel
{
    Q_OBJECT
public:

    HorizonWidget(QWidget* parent);
    void setGeometry(const QRect& r);
    void setRollPitch(float roll, float pitch);
protected:
    virtual void paintEvent(QPaintEvent* event) override;

private:
    float _roll;
    float _pitch;
    QPixmap* _pixmap;
};

#endif //HORIZON_H
