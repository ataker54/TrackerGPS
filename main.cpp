#include <QApplication>
#include "gps_tracker.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    gps_tracker tracker("COM2", 9600);

    return app.exec();
}

