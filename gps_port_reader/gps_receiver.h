#ifndef GPSRECEIVER_H
#define GPSRECEIVER_H

#include <QObject>
#include <QSerialPort>
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <QTimer>
#include <QVector>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>

struct GpsData {
    QString timeUtc;
    QString date;
    double latitude = 0.0;
    double longitude = 0.0;
    double altitude = 0.0;
    double speedKmh = 0.0;
    double course = 0.0;
    int satellites = 0;
    QVector<int> snrValues;
    bool valid = false;
};

Q_DECLARE_METATYPE(GpsData)

class GPSReceiver : public QObject
{
    Q_OBJECT

public:
    explicit GPSReceiver(QObject *parent = nullptr);
    QWidget *widget() const;
    void start(const QString &portName, int baudRate);
    void stop();
    GpsData currentGpsData() const;

    bool isWriteToDebug = true;

signals:
    void gpsUpdated(const GpsData &data);
    void finished();

private slots:
    void attemptReconnect();

private:
    void updateGui(const GpsData &data);
    void readLoop(const QString &portName, int baudRate);
    void parseLine(const QString &line);
    double convertCoord(const QString &coord, const QString &dir);
    GpsData parseGpgga(const QString &line);
    void parseGprmc(const QString &line, GpsData &data);
    void writeToFile(const GpsData &data);

    QString lastPort;
    int lastBaud = 9600;

    bool running = false;
    mutable QMutex mutex;
    GpsData latest;
    QFile logFile;
    QTimer *reconnectTimer = nullptr;
    int reconnectIntervalMs = 5000;

    QFrame *guiFrame = nullptr;
    QLabel *labelStatus = nullptr;
    QLabel *labelCoords = nullptr;
    QPushButton *btnStart = nullptr;
    QPushButton *btnStop = nullptr;
};


#endif // GPSRECEIVER_H



