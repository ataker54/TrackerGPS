#include "gps_tracker.h"
#include "gps_receiver.h"
#include "gps_parser.h"
#include <QDebug>

gps_tracker::gps_tracker(const QString &portName,
                         int baudRate,
                         QObject *parent) : QObject(parent)
{
    receiver = new GPSReceiver(this);
    parser   = new GPSParser(this);

    connect(receiver, &GPSReceiver::getDataReceived,
            parser,   &GPSParser::parseLine);

    connect(parser, &GPSParser::gpsUpdated,
            this,    &gps_tracker::handleParsedData);

    receiver->start("COM2", 9600);
}

void gps_tracker::handleParsedData(const GpsData &data)
{
    qDebug() << data.toString();
    emit gpsUpdated(data);
}











