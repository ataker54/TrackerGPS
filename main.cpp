#include <QApplication>
#include "gps_tracker.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

        gps_tracker tracker;

        QObject::connect(&tracker, &gps_tracker::gpsUpdated,
                         [](const GpsData &data){
                             qDebug() << "[main] Получено:"
                                      << data.latitude << data.longitude;
                         });

        tracker.start("COM6", 9600);

        return app.exec();
}

