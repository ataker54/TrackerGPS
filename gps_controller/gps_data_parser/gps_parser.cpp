#include "gps_parser.h"
#include <QDebug>

GPSParser::GPSParser(QObject *parent) : QObject(parent) {}

void GPSParser::parseLine(const QString &line) {
    if (line.startsWith("$GPGGA")) {
        ggaData = parseGpgga(line);
        gotGGA = true;
    } else if (line.startsWith("$GPRMC")) {
        parseGprmc(line, rmcData);
        gotRMC = true;
    }

    if (gotGGA && gotRMC) {
        latest.latitude = ggaData.latitude;
        latest.longitude = ggaData.longitude;
        latest.altitude = ggaData.altitude;
        latest.satellites = ggaData.satellites;
        latest.timeUtc = rmcData.timeUtc;
        latest.date = rmcData.date;
        latest.speedKmh = rmcData.speedKmh;
        latest.course = rmcData.course;
        latest.valid = ggaData.valid && rmcData.valid;

        emit gpsUpdated(latest);

        gotGGA = false;
        gotRMC = false;
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


