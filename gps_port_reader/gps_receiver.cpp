#include "gps_receiver.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QThread>
#include "gps_port_autodetector.h"

GPSReceiver::GPSReceiver(QObject *parent) : QObject(parent) {}

GPSReceiver::~GPSReceiver()
{
    stop();
}

void GPSReceiver::start(const QString &port_name, int baudRate, bool isWriteToFile, bool isWriteToDebug)
{
    if (isStart || port_name.isEmpty()) return;

    writeFile = isWriteToFile;
    writeDebug = isWriteToDebug;

    serial.setPortName(port_name);
    serial.setBaudRate(baudRate);
    serial.setDataBits(QSerialPort::Data8);
    serial.setParity(QSerialPort::NoParity);
    serial.setStopBits(QSerialPort::OneStop);
    serial.setFlowControl(QSerialPort::NoFlowControl);

    if (!serial.open(QIODevice::ReadOnly)) {
        if (writeDebug) qDebug() << "Не удалось открыть порт" << port_name << ":" << serial.errorString();
        return;
    }

    if (writeFile) {
        logFile.setFileName("gps_data.txt");
        if (!logFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            if (writeDebug) qDebug() << "Не удалось открыть файл gps_data.txt";
            serial.close();
            return;
        }
    }

    buffer.clear();
    fixMap.clear();
    isStart = true;

    connect(&serial, SIGNAL (readyRead()), SLOT(onReadyRead()));

    if (writeDebug) qDebug() << "GPSReceiver started on" << port_name;
}

void GPSReceiver::stop()
{
    if (!isStart) return;
    isStart = false;
    disconnect(&serial, &QSerialPort::readyRead, this, &GPSReceiver::onReadyRead);
    if (logFile.isOpen()) logFile.close();
    if (serial.isOpen()) serial.close();
    emit finished();
    if (writeDebug) qDebug() << "GPSReceiver stopped";
}

GpsData GPSReceiver::currentData() const { return latestData; }

void GPSReceiver::onReadyRead()
{
    if (!isStart) return;

    buffer += serial.readAll();
    QList<QByteArray> lines = buffer.split('\n');
    if (!buffer.endsWith('\n') && !buffer.endsWith('\r')) {
        buffer = lines.takeLast();
    } else {
        buffer.clear();
    }

    for (const QByteArray &raw : lines) {
        QString line = QString::fromLatin1(raw).trimmed();
        if (line.isEmpty()) continue;

        GpsData data;
        if (line.contains("$GPGGA") || line.contains("GGA"))
            data = parseGpgga(line);
        else if (line.contains("$GPRMC") || line.contains("RMC"))
            parseGprmc(line, data);
        else
            continue;

        if (!data.valid) continue;

        latestData = data;
        emit gpsDataUpdated(data);

        if (writeDebug) {
            qDebug().nospace() << "GPS:"
                               << " UTC:" << data.timeUtc
                               << " Lat:" << QString::number(data.latitude, 'f', 6)
                               << " Lon:" << QString::number(data.longitude, 'f', 6)
                               << " Alt:" << QString::number(data.altitude, 'f', 2);
        }

        if (writeFile) writeToFile(data);
    }
}

//преобразование NMEA данных в десятичный формат
double GPSReceiver::convertCoord(const QString &coord, const QString &dir)
{
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

//парсинг строк $GPGGA
GpsData GPSReceiver::parseGpgga(const QString &line)
{
    GpsData data;
    QStringList parts = line.split(",");
    if (parts.size() < 10)
        return data;

    data.latitude = convertCoord(parts[2], parts[3]);
    data.longitude = convertCoord(parts[4], parts[5]);
    data.altitude = parts[9].toDouble();
    data.timeUtc = parts[1];
    data.valid = (parts[6] != "0");
    return data;
}

//парсинг строк $GPRMC
void GPSReceiver::parseGprmc(const QString &line, GpsData &data)
{
    QStringList parts = line.split(",");
    if (parts.size() < 9)
        return;

    if (parts[2] == "A") {
        data.latitude = convertCoord(parts[3], parts[4]);
        data.longitude = convertCoord(parts[5], parts[6]);
        data.speedKmh = parts[7].toDouble() * 1.852;
        data.timeUtc = parts[1];
        data.valid = true;
    }
}

//функция для читаемого вывода
void GPSReceiver::writeToFile(const GpsData &data)
{
    static QMap<QString, GpsData> fixMap;

    GpsData &entry = fixMap[data.timeUtc];
    if (data.altitude != 0.0)
        entry.altitude = data.altitude;
    if (data.speedKmh != 0.0)
        entry.speedKmh = data.speedKmh;
    if (data.course != 0.0)
           entry.course = data.course;
    if (data.satellites != 0)
           entry.satellites = data.satellites;
    if (data.hdop != 0.0)
           entry.hdop = data.hdop;
    if (!data.date.isEmpty())
           entry.date = data.date;
    entry.latitude = data.latitude;
    entry.longitude = data.longitude;
    entry.timeUtc = data.timeUtc;
    entry.valid = true;

    if (entry.altitude != 0.0 && entry.speedKmh != 0.0) {
        QTextStream out(&logFile);
        out << "GPS Fix @ " << entry.timeUtc << "\n"
            << "Latitude   : " << QString::number(entry.latitude, 'f', 6) << "\n"
            << "Longitude  : " << QString::number(entry.longitude, 'f', 6) << "\n"
            << "Altitude   : " << QString::number(entry.altitude, 'f', 2) << " m\n"
            << "Speed      : " << QString::number(entry.speedKmh, 'f', 2) << " km/h\n\n"
            << "Course      : " << QString::number(entry.course, 'f', 2) << "°\n"
            << "Satellites  : " << entry.satellites << "\n"
            << "HDOP        : " << QString::number(entry.hdop, 'f', 2) << "\n\n";

        fixMap.remove(entry.timeUtc);
    }
}

