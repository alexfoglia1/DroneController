#ifndef PLOT_H
#define PLOT_H

#include <qlabel.h>
#include <qpixmap.h>
#include <vector>

#define OFFSET_X_PX 10

enum class PlotTrack
{
    PLOT_TRACK_1 = 0,
    PLOT_TRACK_2,
    PLOT_TRACK_3,
    PLOT_TRACK_4
};

class PlotWidget :
    public QLabel
{
    Q_OBJECT
public:

    PlotWidget(QWidget* parent);
    void setGeometry(const QRect& r);
    void addValue(PlotTrack track, float val);
    void setRange(PlotTrack track, float range);
    const QColor trackColors[4] = { Qt::cyan, Qt::red, Qt::magenta, Qt::green };
protected:
    virtual void paintEvent(QPaintEvent* event) override;

private:
    QPixmap* pixmap;
    std::vector<float> track1;
    std::vector<float> track2;
    std::vector<float> track3;
    std::vector<float> track4;
    float track1_range;
    float track2_range;
    float track3_range;
    float track4_range;
    float t_range;
};

#endif //PLOT_H
