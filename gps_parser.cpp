#include "gps_parser.h"
#include <QDebug>

GPSParser::GPSParser(QObject *parent) : QObject(parent) {}

void GPSParser::parseLine(const QString &line) {
    //qDebug() << "parseLine:" << line;

    if (line.startsWith("$GPGGA")) {
        GpsData parsed = parseGpgga(line);
        latest = parsed;
        emit gpsUpdated(latest);
    }
    else if (line.startsWith("$GPRMC")) {
        parseGprmc(line, latest);
        emit gpsUpdated(latest);
    }
    else if (line.startsWith("$GPGLL")) {
        QStringList parts = line.split(",");
        if (parts.size() >= 7 && parts[6] == "A") {
            latest.latitude = convertCoord(parts[1], parts[2]);
            latest.longitude = convertCoord(parts[3], parts[4]);
            latest.timeUtc = parts[5];
            latest.valid = true;
            emit gpsUpdated(latest);
        }
    }
}

double GPSParser::convertCoord(const QString &coord, const QString &dir) {
    if (coord.isEmpty() || (dir != "N" && dir != "S" && dir != "E" && dir != "W"))
        return 0.0;

    int degLen = (dir == "N" || dir == "S") ? 2 : 3;
    int deg = coord.left(degLen).toInt();
    double min = coord.mid(degLen).toDouble();
    double decimal = deg + min / 60.0;
    if (dir == "S" || dir == "W")
        decimal *= -1;
    return decimal;
}

GpsData GPSParser::parseGpgga(const QString &line) {
    GpsData data;
    QStringList parts = line.split(",");
    if (parts.size() < 10)
        return data;

    data.latitude = convertCoord(parts[2], parts[3]);
    data.longitude = convertCoord(parts[4], parts[5]);
    data.altitude = parts[9].toDouble();
    data.timeUtc = parts[1];
    data.valid = (parts[6] != "0");
    data.satellites = parts[7].toInt();
    return data;
}

void GPSParser::parseGprmc(const QString &line, GpsData &data) {
    QStringList parts = line.split(",");
    if (parts.size() < 9)
        return;

    if (parts[2] == "A") {
        data.latitude = convertCoord(parts[3], parts[4]);
        data.longitude = convertCoord(parts[5], parts[6]);
        data.speedKmh = parts[7].toDouble() * 1.852;
        data.course = parts[8].toDouble();
        data.timeUtc = parts[1];
        data.date = parts[9];
        data.valid = true;
    }
}
