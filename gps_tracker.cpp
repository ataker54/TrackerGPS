#include "gps_tracker.h"
#include "gps_receiver.h"
#include "gps_parser.h"
#include <QDebug>

gps_tracker::gps_tracker(QObject *parent) : QObject(parent)
{
    receiver = new GPSReceiver(this);
    parser   = new GPSParser(this);

    connect(receiver, &GPSReceiver::GetDataReceived,
            this, &gps_tracker::handleRawData);

    connect(parser, &GPSParser::gpsUpdated,
            this, &gps_tracker::handleParsedData);
}

void gps_tracker::start(const QString &portName, int baudRate)
{
    receiver->start(portName, baudRate);
}

void gps_tracker::stop()
{
    receiver->stop();
}

void gps_tracker::handleRawData(const QByteArray &chunk)
{
    const QString line = QString::fromUtf8(chunk).trimmed();
    if (!line.isEmpty()) {
        parser->parseLine(line);
    }
}

void gps_tracker::handleParsedData(const GpsData &data)
{
    qDebug() << "[gps_tracker]" << data.toString();
    emit gpsUpdated(data);
}











