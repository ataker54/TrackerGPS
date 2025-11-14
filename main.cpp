#include <QApplication>
#include "gps_tracker.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    gps_tracker tracker;
    tracker.show();
    qDebug() << "main started";
    return app.exec();
}

