#include <QApplication>
#include "gps_tracker.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    qRegisterMetaType<GpsData>("GpsData");
    QApplication app(argc, argv);
    gps_tracker tracker;
    tracker.setWindowTitle("GPS Tracker");
    tracker.resize(400, 300);
    tracker.show();
    qDebug() << "main started";
    return app.exec();
}

