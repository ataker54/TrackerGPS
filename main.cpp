#include <QApplication>
#include "gps_controller.h"
#include <QDebug>
#include <gps_port_autodetector.h>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    gps_controller controller;

    QObject::connect(&controller, &gps_controller::gpsUpdated,
                     [](const GpsData &data){
                         qDebug() << "[main]" << data.toString();
                     });

    controller.start("", 9600);

    return app.exec();
}


