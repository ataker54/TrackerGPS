#include "gps_tracker.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //1 autodetector

    gps_tracker w;
    w.show();
    return a.exec();
}
