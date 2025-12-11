#include <QApplication>
#include "gps_tracker.h"
#include <QDebug>
#include <gps_port_autodetector.h>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qRegisterMetaType<GpsData>("GpsData");

    gps_tracker tracker("", 9600);

    QObject::connect(&tracker, &gps_tracker::gpsUpdated,
                     [](const GpsData &data){
                         qDebug() << "[main]" << data.toString();
                     });

    return app.exec();
}

