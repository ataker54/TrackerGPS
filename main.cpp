#include <QApplication>
#include "gps_tracker.h"

int main(int argc, char *argv[])
{
    qRegisterMetaType<GpsData>("GpsData");
    QApplication app(argc, argv);
    gps_tracker tracker;
    tracker.show();
    return app.exec();
}

