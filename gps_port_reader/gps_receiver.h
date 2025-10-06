#ifndef GPSRECEIVER_H
#define GPSRECEIVER_H

#include <QObject>
#include <QSerialPort>
#include <QFile>
#include <QTextStream>


struct GpsData {
    double latitude = 0.0;
    double longitude = 0.0;
    double altitude = 0.0;
    double speedKmh = 0.0;
    QString timeUtc;
    bool valid = false;
};

class GPSReceiver : public QObject
{
    Q_OBJECT

public:
    explicit GPSReceiver(QObject *parent = nullptr);
    void start(const QString &port_name,int baudRate = 9600, int durationMs = 30000);

private:
    double convertCoord(const QString &coord, const QString &dir);
    GpsData parseGpgga(const QString &line);
    void parseGprmc(const QString &line, GpsData &data);
    void writeToFile(const GpsData &data);

    QSerialPort serial;
    QFile logFile;
    bool headerWritten = false;
};

#endif // GPSRECEIVER_H


