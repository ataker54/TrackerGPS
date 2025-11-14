#include "gps_tracker.h"
#include <QDebug>

gps_tracker::gps_tracker(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    receiver = new GPSReceiver(this);
    layout->addWidget(receiver->widget());


}

gps_tracker::~gps_tracker()
{
    receiver->stop();
}









