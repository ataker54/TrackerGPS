#include "gps_tracker.h"

#include "ui_gps_tracker.h"
#include <QtDebug>
#include <QSerialPortInfo>
#include <QString>
#include <gps_port_autodetector.h>

gps_tracker::gps_tracker(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::gps_tracker)
{
    ui->setupUi(this);
    connect(ui->btn, SIGNAL(pressed()),this,SLOT(btn_click()));
}

gps_tracker::~gps_tracker()
{
    delete ui;
}



void gps_tracker::btn_click()
{
    GpsPortAutoDetector detector;
    detector.FindPorts();

    QString gpsPort = detector.getGpsPortName();
    if (gpsPort.isEmpty()) {
        qDebug() << "GPS-порт не найден.";
        return;
    }
    GPSReceiver *receiver = new GPSReceiver(this);
    receiver->start(gpsPort, 9600);
}

