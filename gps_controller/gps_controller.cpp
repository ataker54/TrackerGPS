#include "gps_controller.h"
#include "gps_receiver.h"
#include "gps_parser.h"
#include "gps_port_autodetector.h"
#include <QDebug>

gps_controller::gps_controller(QObject *parent) : QObject(parent)
{
    detector = new GpsPortAutoDetector(this);
    receiver = new GPSReceiver(this);
    parser   = new GPSParser(this);

    connect(receiver, &GPSReceiver::getDataReceived,
            parser,   &GPSParser::parseLine);

    connect(parser, &GPSParser::gpsUpdated,
            this,    &gps_controller::handleParsedData);
}

void gps_controller::start(const QString &portName, int baudRate)
{
    detector->FindPorts();
    receiver->start(portName, baudRate);
}
void gps_controller::stop()
{
    receiver->stop();
}

void gps_controller::handleParsedData(const GpsData &data)
{
    //qDebug() << "[gps_tracker]" << data.toString();
    emit gpsUpdated(data);
}












