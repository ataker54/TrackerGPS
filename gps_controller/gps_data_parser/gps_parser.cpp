#include "gps_parser.h"
#include <QDebug>

GPSParser::GPSParser(QObject *parent) : QObject(parent) {}

void GPSParser::parseLine(const QString &line) {
    if (line.startsWith("$GPGGA")) {
        ggaData = parseGpgga(line);
    } else if (line.startsWith("$GPRMC")) {
        parseGprmc(line);
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

    latest.latitude = convertCoord(parts[2], parts[3]);
    latest.longitude = convertCoord(parts[4], parts[5]);
    latest.altitude = parts[9].toDouble();
    latest.timeUtc = parts[1];
    latest.valid = (parts[6] != "0");
    latest.satellites = parts[7].toInt();
    ggaData = latest;

    if (rmcData.valid) {
        latest.date = rmcData.date;
        latest.speedKmh = rmcData.speedKmh;
        latest.course = rmcData.course;
        latest.valid = latest.valid && rmcData.valid;
        emit gpsUpdated(latest);
    }
    return latest;
}

void GPSParser::parseGprmc(const QString &line) {
    QStringList parts = line.split(",");
    if (parts.size() < 9)
        return;

    if (parts[2] == "A") {
        latest.latitude = convertCoord(parts[3], parts[4]);
        latest.longitude = convertCoord(parts[5], parts[6]);
        latest.speedKmh = parts[7].toDouble() * 1.852;
        latest.course = parts[8].toDouble();
        latest.timeUtc = parts[1];
        latest.date = parts[9];
        latest.valid = true;
        rmcData = latest;

        if (ggaData.valid) {
            latest.altitude = ggaData.altitude;
            latest.satellites = ggaData.satellites;
            latest.valid = latest.valid && ggaData.valid;
            emit gpsUpdated(latest);
    }

}
}

