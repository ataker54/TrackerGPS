#pragma once
#include <QObject>
#include "gps_data.h"

class GPSParser : public QObject {
    Q_OBJECT
public:
    explicit GPSParser(QObject *parent = nullptr);

    void parseLine(const QString &line);

signals:
    void gpsUpdated(const GpsData &data);

private:
    double convertCoord(const QString &coord, const QString &dir);
    GpsData parseGpgga(const QString &line);
    void parseGprmc(const QString &line, GpsData &data);

    GpsData ggaData;
    GpsData rmcData;
    GpsData latest;

    bool gotGGA = false;
    bool gotRMC = false;
};
