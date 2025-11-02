#ifndef GPSRECEIVER_H
#define GPSRECEIVER_H

#include <QObject>
#include <QSerialPort>
#include <QFile>
#include <QTextStream>


struct GpsData { // fix: только это можно получить? А Number of satellites? Мб что-то еще полезное
    double latitude = 0.0;
    double longitude = 0.0;
    double altitude = 0.0;
    double speedKmh = 0.0;
    double course = 0.0;
    int satellites = 0;
    double hdop = 0.0;
    QString timeUtc;
    QString date;
    bool valid = false;

};

class GPSReceiver : public QObject
{
    Q_OBJECT

public:
    explicit GPSReceiver(QObject *parent = nullptr);
    ~GPSReceiver() override;
    void start(const QString &port_name,
               int baudRate = 9600,
               bool isWriteToFile = false,
               bool isWriteToDebug = true);
    void stop();
    GpsData currentData() const;

signals:
    void gpsDataUpdated(const GpsData& data);
    void finished();
private slots:
    void onReadyRead();
// fix: Договаривались, что должны быть еще: 1)функция получения актуального значения типа GpsData.
//    2)Сигнал, который постоянно эмитится при обновлении данных от GPS. Эмитится актуальный GpsData
    // 3) раз есть void start(...), должен быть void stop()
private:
    double convertCoord(const QString &coord,
                        const QString &dir);
    GpsData parseGpgga(const QString &line);
    void parseGprmc(const QString &line,
                    GpsData &data);
    void writeToFile(const GpsData &data);

    QSerialPort serial;
    QFile logFile;
    QByteArray buffer;
    GpsData latestData;
    bool isStart = false;
    bool writeFile = false;
    bool writeDebug = true;
    QMap<QString, GpsData> fixMap;
};

#endif // GPSRECEIVER_H


