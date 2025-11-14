#include "gps_tracker.h"
#include <QDebug>

gps_tracker::gps_tracker(QWidget *parent) : QWidget(parent)
{
    GPSReceiver *receiver = new GPSReceiver;

    QObject::connect(receiver, &GPSReceiver::GetDataReceived,
                     [](const QByteArray &data){
        qDebug() << "Полученные данные:" << data;
    });

    receiver->start("COM2", 9600);
}

gps_tracker::~gps_tracker()
{

}









